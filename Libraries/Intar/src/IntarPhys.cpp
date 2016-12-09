/**
 * @file       IntarPhys.cpp
 * @brief      Library for the IR physical layer in Arduino-based laser tag
 * @author     Shawn Hymel
 * @copyright  2015 Shawn Hymel
 * @license    http://opensource.org/licenses/MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <Arduino.h>
 
#include "IntarPhys.h"

// We need to create a global instance so that the ISR knows what to talk to
IntarPhys Intar_Phys;

/**
 * @brief Constructor
 */
IntarPhys::IntarPhys()
{
    
}

/**
 * @brief Destructor
 */
IntarPhys::~IntarPhys()
{
    
}

/**
 * @brief Configures the parameters for the IntarPhys object. Call this first.
 *
 * @param[in] rx_pin Receive pin to use.
 * @return True on initialization success. False on failure.
 */
bool IntarPhys::begin(uint8_t recv_pin /*= 0*/)
{
    _recv_pin = recv_pin;
    
    // Set up the hardware PWM (specific to processor) for the IR LED
#if defined(__AVR_ATmega328P__)
    
    // Use Timer 2 in Fast PWM mode, clear on match
    TCCR2A = _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
    
    // Reset timer on TOP (OCR2A), prescaler = 8
    TCCR2B = _BV(WGM22) | _BV(CS21);
    
    // TOP (OCR2A) is modulation frequency. Timer 2 resets on this number.
    OCR2A = MOD_COUNTER_VAL >> 3;   // Account for prescaler of 8
    
    // Set the PWM value to 50%
    OCR2B = MOD_COUNTER_VAL >> 4;   // #define won't divide nicely
    
    // Enable Timer 2 overflow interrupt
    TIMSK2 = _BV(TOIE2);
    
    // Enable global interrupts
    sei();

#elif defined(__AVR_ATtiny84__)

  // Use Timer 1 in Fast PWM mode, clear on match
  TCCR1A = _BV(WGM11) | _BV(WGM10);
  
  // Reset timer on TOP (OCR1A), prescaler = 8
  TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS11);

  // TOP (OCR1A) is modulation frequency. Timer 1 resets on this number.
  OCR1A = MOD_COUNTER_VAL >> 3;   // Account for prescaler of 8

  // Set the PWM value to 50%
  OCR1B = MOD_COUNTER_VAL >> 4;   // #define won't divide nicely

  // Enable Timer 1 overflow interrupt
  TIMSK1 = _BV(TOIE1);

  // Enable global interrupts
  sei();
    
#elif defined(KINETISL)

    // Set overflow value
    FTM0_SC = 0;
    FTM0_CNT = 0;
    FTM0_MOD = MOD_COUNTER_VAL;
    
    // Enable TOIE and set prescaler to 1
    FTM0_SC = 0b11001000;
    
    // Enable PWM with set on reload and clear on match
    FTM0_C4SC = 0;
    delayMicroseconds(1);
    FTM0_C4SC = 0b10101000;
    
    // We want 50% duty cycle PWM
    FTM0_C4V = FTM0_MOD / 2;
    
    // Set urgency of interrupt (0, 64, 128, or 192)
    NVIC_SET_PRIORITY(IRQ_FTM0, 64);
    
    // Enable interrupt vector
    NVIC_ENABLE_IRQ(IRQ_FTM0);
    
#else
# error Processor not supported
#endif

    // Disable and transmitter receiver by default
    _xmit_enabled = false;
    _recv_enabled = false;
    
    return true;
}

/**
 * @brief Turn on the IR LED transmitter
 */
void IntarPhys::enableTransmitter()
{
    // Set LED as output and turn off initially
    pinMode(IR_LED_PIN, OUTPUT);
    pulse(false);
    
    // Initialize transmission members
    _xmit_tick_counter = 0;
    _xmit_block_counter = 0;
    _xmit_state = XMIT_STATE_WAITING;
    _bit_pulse = true;
    _xmit_bit = 0;
    _xmit_byte = 0;
    _bytes_to_send = 0;
    _xmit_ptr = -1;
    
    // Enable transmitter
    _xmit_enabled = true;
}

/**
 * @brief Turn off the transmitter
 */
void IntarPhys::disableTransmitter()
{
    _xmit_enabled = false;
}

/**
 * @brief Turn on the receiver
 */
void IntarPhys::enableReceiver()
{   
    // Reset receiver parameters
    _recv_state = RECV_STATE_IDLE;
    _recv_tick_counter = 0;
    _recv_sample_counter = 0;
    _recv_bit_ptr = 0;
    _recv_byte_ptr = 0;
    _recv_head = 0;
    _recv_tail = 0;
    _recv_ring_overflow = false;

    // Clear received bytes array
    memset(_recv_bytes, 0, RECV_MAX_PACKETS);
    
    // Enable receiver
    _recv_enabled = true;
}

/**
 * @brief Turn off the receiver
 */
void IntarPhys::disableReceiver()
{
    _recv_enabled = false;
}

/**
 * @brief Transmit a packet over IR
 *
 * @param[in] data Data to send over IR
 * @param[in] len Length (in bytes) of data
 */
void IntarPhys::xmit(uint8_t data[], uint8_t len)
{
    // Wait for previous transmission to finish
    flushXmit();
    
    // Copy data to transmit buffer
    memcpy(_xmit_buf, data, len);
    
    // Reset global variables (e.g. counters)
    _xmit_block_counter = 0;
    _bit_pulse = true;
    _xmit_bit = 0;
    _xmit_byte = 0;
    _xmit_ptr = 0;
    
    // Set up flags and data counter
    _bytes_to_send = len;
    _xmit_state = XMIT_STATE_SOM_PULSE;
}

/**
 * @brief Determines if there is data in the receive buffer ready to be read
 *
 * @return number of packets waiting to be read
 */
uint8_t IntarPhys::available()
{
    return (_recv_head + RECV_MAX_PACKETS - _recv_tail) % RECV_MAX_PACKETS;
}

/**
 * @brief Determines if the receiver ring buffer has overflowed
 *
 * @return True for overflow. False for all normal.
 */
bool IntarPhys::overflow()
{
    return _recv_ring_overflow;
}

/**
 * @brief Read the next packet from the ring buffer
 *
 * @param[out] packet pointer to buffer to store the read packet
 * @return RECV_ERROR (-1) on error. Number of bytes in packet otherwise.
 */
uint8_t IntarPhys::read(uint8_t packet[MAX_PACKET_SIZE])
{
    uint8_t num_bytes;
    
    // Return if empty buffer
    if ( _recv_head == _recv_tail ) {
        return 0;
    }
    
    // Read in number of bytes in packet
    num_bytes = _recv_bytes[_recv_tail];
    
    // Copy packet and update tail
    if ( num_bytes != RECV_ERROR ) {
        memcpy(packet, _recv_buffer + (_recv_tail * MAX_PACKET_SIZE), 
                                                            MAX_PACKET_SIZE);
    }
    _recv_tail = (_recv_tail + 1) % RECV_MAX_PACKETS;
    
    // If we had a packet error, return with that
    if ( num_bytes == RECV_ERROR ) {
        return RECV_ERROR;
    }
    
    // If we had a ring buffer overflow, clear the flag
    if ( _recv_ring_overflow ) {
        _recv_ring_overflow = false;
    }
    
    return num_bytes;
}
 
/**
 * @brief Wait until the current IR transmission finished
 */
void IntarPhys::flushXmit()
{
    while ( _xmit_state != XMIT_STATE_WAITING ) {
        delayMicroseconds(1);
    }
}

/**
 * @brief Transmit IR packets in a non-blocking manner
 */
void IntarPhys::doXmit()
{
    // State machine for transmitting a message
    switch ( _xmit_state ) {
    
        // If we are in the waiting state, do nothing
        case XMIT_STATE_WAITING:
            break;
            
        // Start with SOF pulse
        case XMIT_STATE_SOM_PULSE:
            _xmit_block_counter--;
            if ( _xmit_block_counter <= -1 ) {
                pulse(true);
                _xmit_block_counter = SOM_PULSE_BLOCKS - 1;
            }
            if ( _xmit_block_counter == 0 ) {
                _xmit_state = XMIT_STATE_SOM_SPACE;
            }
            break;
            
        // Space before message
        case XMIT_STATE_SOM_SPACE:
            _xmit_block_counter--;
            if ( _xmit_block_counter <= -1 ) {
                pulse(false);
                _xmit_block_counter = SOM_SPACE_BLOCKS - 1;
            }
            if ( _xmit_block_counter == 0 ) {
                _xmit_state = XMIT_STATE_MSG;
            }
            break;
            
        // Transmit the message
        case XMIT_STATE_MSG:
            _xmit_block_counter--;
            if ( _xmit_block_counter <= -1 ) {
            
                // Transmit pulse on new bit and move to new byte at end of bits
                if ( _bit_pulse ) {
                    if ( _xmit_bit == 0 ) {
                        _xmit_byte = _xmit_buf[_xmit_ptr];
                    }
                    pulse(true);
                    if ( _xmit_byte & (0x80 >> _xmit_bit) ) {
                        _xmit_block_counter = ONE_PULSE_BLOCKS - 1;
                    } else {
                        _xmit_block_counter = ZERO_PULSE_BLOCKS - 1;
                    }
                    
                // Transmit different space for '0' or '1'
                } else {
                    pulse(false);
                    if ( _xmit_byte & (0x80 >> _xmit_bit) ) {
                        _xmit_block_counter = ONE_SPACE_BLOCKS - 1;
                    } else {
                        _xmit_block_counter = ZERO_SPACE_BLOCKS - 1;
                    }
                }
            }
            
            // Move from pulse to space or space to new bit
            if ( _xmit_block_counter == 0 ) {
                _bit_pulse = !_bit_pulse;
                if ( _bit_pulse ) {
                    _xmit_bit++;
                    if ( _xmit_bit >= BITS_PER_BYTE ) {
                        _xmit_ptr++;
                        _xmit_bit = 0;
                        if ( (_xmit_ptr >= _bytes_to_send) ||
                             (_xmit_ptr >= XMIT_BUF_SIZE) ) {
                            _xmit_state = XMIT_STATE_EOM_PULSE;
                        }
                    }
                }
            }
            break;
            
        // Send out EOM pulse
        case XMIT_STATE_EOM_PULSE:
            _xmit_block_counter--;
            if ( _xmit_block_counter <= -1 ) {
                pulse(true);
                _xmit_block_counter = EOM_PULSE_BLOCKS - 1;
            }
            if ( _xmit_block_counter == 0 ) {
                _xmit_state = XMIT_STATE_EOM_SPACE;
            }
            break;
            
        // Force space after EOM pulse
        case XMIT_STATE_EOM_SPACE:
            _xmit_block_counter--;
            if ( _xmit_block_counter <= -1 ) {
                pulse(false);
                _xmit_block_counter = EOM_SPACE_BLOCKS - 1;
            }
            if ( _xmit_block_counter == 0 ) {
                _xmit_state = XMIT_STATE_WAITING;
            }
            break;
            
        // Unknown state! Switch to waiting state
        default:
            _xmit_state = XMIT_STATE_WAITING;
            break;
    }
}

/**
 * @brief Turn on the IR LED with modulation
 *
 * @param on True to turn on LED, false to turn it off.
 */
void IntarPhys::pulse(bool on)
{
    if ( on ) {
#if defined(__AVR_ATmega328P__)
        TCCR2A |= _BV(COM2B1);
#elif defined(__AVR_ATtiny84__)
        TCCR1A |= _BV(COM1B1);
#elif defined(KINETISL)
        *portConfigRegister(IR_LED_PIN) = PORT_PCR_MUX(4);
#endif
    } else {
#if defined(__AVR_ATmega328P__)
        TCCR2A &= ~(_BV(COM2B1));
#elif defined(__AVR_ATtiny84__)
        TCCR1A &= ~(_BV(COM1B1));
#elif defined(KINETISL)
        *portConfigRegister(IR_LED_PIN) = PORT_PCR_MUX(1);
#endif
    }
}

/**
 * @brief Writes a 1 or 0 to the receiver buffer
 */
void IntarPhys::storeBit(uint8_t recv_bit)
{
    uint8_t current_byte;
    
    // Store bit
    current_byte = _recv_buffer[(_recv_head * MAX_PACKET_SIZE) + 
                                                        _recv_byte_ptr];
    if ( recv_bit ) {
        _recv_buffer[(_recv_head * MAX_PACKET_SIZE) + _recv_byte_ptr] =
                                    current_byte | (0x80 >> _recv_bit_ptr);
    } else {
        _recv_buffer[(_recv_head * MAX_PACKET_SIZE) + _recv_byte_ptr] =
                                    current_byte & ~(0x80 >> _recv_bit_ptr);
    }
    
    // Increase pointers
    _recv_bit_ptr++;
    if ( _recv_bit_ptr >= BITS_PER_BYTE ) {
        _recv_bit_ptr = 0;
        _recv_byte_ptr++;
        
        // Check for packet overflow
        if ( _recv_byte_ptr >= MAX_PACKET_SIZE ) {
            handleRecvError();
            _recv_state = RECV_STATE_IDLE;
        }
    }
}

/**
 * @brief Stores number of bytes received and increments head
 */
void IntarPhys::storePacket()
{  
    // Store number of bytes received and increment ring buffer head
    if ( (_recv_head + 1) % RECV_MAX_PACKETS != _recv_tail ) {
        _recv_bytes[_recv_head] = _recv_byte_ptr;
        _recv_head = (_recv_head + 1) % RECV_MAX_PACKETS;
    } else {
        _recv_ring_overflow = true;
    }
}

/**
 * @brief Stores error in "number of bytes" array and increments head
 */
void IntarPhys::handleRecvError()
{
    // Store error and increment ring buffer head
    if ( (_recv_head + 1) % RECV_MAX_PACKETS != _recv_tail ) {
        _recv_bytes[_recv_head] = RECV_ERROR;
        _recv_head = (_recv_head + 1) % RECV_MAX_PACKETS;
    } else {
        _recv_ring_overflow = true;
    }
}

/**
 * @brief Receive IR packets asynchronously and store them in a buffer
 */
void IntarPhys::doRecv()
{
    uint8_t ir;
    
    // Increment the sample counter every time we sample
    _recv_sample_counter++;
    if ( _recv_sample_counter >= RECV_SAMPLE_MAX ) {
        _recv_sample_counter = RECV_SAMPLE_MAX;
    }
    
    // Sample the receiver
    ir = (uint8_t)digitalRead(_recv_pin);
    
    // The receiver state machine
    switch ( _recv_state ) {
        
        // Do nothing. If we see an edge, start recording.
        case RECV_STATE_IDLE:
            if ( ir == RECV_PULSE ) {
                _recv_sample_counter = 0;
                _recv_state = RECV_STATE_SOM_PULSE;
            }
            break;
            
        // Wait until th end of the pulse and check if it's a SOF pulse
        case RECV_STATE_SOM_PULSE:
            if ( ir == RECV_SPACE ) {
                
                // If pulse length is acceptable, move on to SOF space
                if ( (_recv_sample_counter >= RECV_SOM_PULSE_MIN) &&
                     ( _recv_sample_counter <= RECV_SOM_PULSE_MAX) ) {
                    _recv_sample_counter = 0;
                    _recv_state = RECV_STATE_SOM_SPACE;
                } else {
                    _recv_sample_counter = 0;
                    _recv_state = RECV_STATE_IDLE;
                }
            }
            break;
            
        // Wait until end of SOF space
        case RECV_STATE_SOM_SPACE:
            if ( ir == RECV_PULSE ) {
                
                // If space length is acceptable, move on to hit pulse
                if ( (_recv_sample_counter >= RECV_SOM_SPACE_MIN) &&
                      (_recv_sample_counter <= RECV_SOM_SPACE_MAX) ) {
                    _recv_byte_ptr = 0;
                    _recv_bit_ptr = 0;
                    _recv_sample_counter = 0;
                    _recv_state = RECV_STATE_BIT_PULSE;
                } else {
                    _recv_sample_counter = 0;
                    _recv_state = RECV_STATE_IDLE;
                }
            }
            break;
            
        // Wait until the end of the bit pulse
        case RECV_STATE_BIT_PULSE:
            if ( ir == RECV_SPACE ) {
                
                // If the bit pulse length is acceptable, move on to bit space
                if ( (_recv_sample_counter >= RECV_BIT_PULSE_MIN) &&
                     (_recv_sample_counter <= RECV_BIT_PULSE_MAX) ) {
                    _recv_sample_counter = 0;
                    _recv_state = RECV_STATE_BIT_SPACE;
                } else if ( (_recv_sample_counter >= RECV_SOM_PULSE_MIN) &&
                            (_recv_sample_counter <= RECV_SOM_PULSE_MAX) ) {
                    storePacket();
                    _recv_sample_counter = 0;
                    _recv_state = RECV_STATE_SOM_SPACE;
                } else {
                    handleRecvError();
                    _recv_sample_counter = 0;
                    _recv_state = RECV_STATE_IDLE;
                }
            }
            break;
            
        // Wait until end of bit space and store bit (or end receive mode)
        case RECV_STATE_BIT_SPACE:
            if ( ir == RECV_PULSE ) {
                
                // Determine the bit based on the space
                if ( (_recv_sample_counter >= RECV_ZERO_SPACE_MIN) &&
                     (_recv_sample_counter <= RECV_ZERO_SPACE_MAX) ) {
                    storeBit(0);
                    _recv_sample_counter = 0;
                    _recv_state = RECV_STATE_BIT_PULSE;
                } else if ( (_recv_sample_counter >= RECV_ONE_SPACE_MIN) &&
                            (_recv_sample_counter <= RECV_ONE_SPACE_MAX) ) {
                    storeBit(1);
                    _recv_sample_counter = 0;
                    _recv_state = RECV_STATE_BIT_PULSE;
                } else {
                    handleRecvError();
                    _recv_sample_counter = 0;
                    _recv_state = RECV_STATE_IDLE;
                }
            } else if ( _recv_sample_counter >= RECV_IDLE_SPACE_MIN ) {
                storePacket();
                _recv_sample_counter = 0;
                _recv_state = RECV_STATE_IDLE;
            }
            break;
            
        // Unknown state. Store error and go to idle state.
        default:
            handleRecvError();
            _recv_sample_counter = 0;
            _recv_state = RECV_STATE_IDLE;
    }
}

/**
 * @brief ISR for IntarPhys
 *
 * Handle transmitting and receiving IR data. Each overflow is a "tick." Each
 * lowest division of the IR protocol is a "block." 
 */
void IntarPhys::isr()
{
    
    // Measure ticks. Every block, perform the transmit function.
    _xmit_tick_counter++;
    if ( _xmit_enabled && (_xmit_tick_counter >= XMIT_TICKS_PER_BLOCK) ) {
        _xmit_tick_counter = 0;
        doXmit();
    }
    
    // If our receiver is enabled, count to receiver sample
    _recv_tick_counter++;
    if ( _recv_enabled && (_recv_tick_counter >= RECV_TICKS_PER_SAMPLE) ) {
        _recv_tick_counter = 0;
        doRecv();
    }
}

/**
 * @brief Global interrupt service routine for timer
 *
 * We define ISR here to allow making calls to functions in the IntarPhys class.
 * To do this, we instantiate a IntarPhys object globally in the .cpp file.
 **/
#if defined(__AVR_ATmega328P__)
ISR(TIMER2_OVF_vect)
{
    Intar_Phys.isr();
}
#elif defined(__AVR_ATtiny84__)
ISR(TIM1_OVF_vect)
{
    Intar_Phys.isr();
}
#elif defined(KINETISL)
void ftm0_isr(void)
{
    // Clear timer overflow flags
    FTM0_SC |= (1 << 7);
    
    // Execute Intar IR's ISR
    Intar_Phys.isr();
}
#endif