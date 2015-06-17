/**
 * @file       IntarIR.cpp
 * @brief 	   Library for the Arduino-based laser tag system
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
 
#include "IntarIR.h"

// We need to create a global instance so that the ISR knows what to talk to
IntarIR Intar_IR;

/**
 * @brief Constructor
 */
IntarIR::IntarIR()
{
    // Initialize transmission members
    _xmit_tick_counter = 0;
    _xmit_block_counter = 0;
    _xmit_state = XMIT_STATE_WAITING;
    _bit_pulse = true;
    _xmit_bit = 0;
    _xmit_byte = 0;
    _bytes_to_send = 0;
    _xmit_ptr = -1;
}

/**
 * @brief Destructor
 */
IntarIR::~IntarIR()
{
    
}

/**
 * @brief Configures the parameters for the IntarIR object. Call this first.
 *
 * @param tx_pin Transmit pin to use.
 * @param rx_pin Receive pin to use.
 * @return True on initialization success. False on failure.
 */
bool IntarIR::begin(uint8_t recv_pin /*= 0*/)
{
    pinMode(IR_LED_PIN, OUTPUT);
    _recv_pin = recv_pin;
    
    // Set up the hardware PWM for the IR LED
#if defined(__AVR_ATmega328P__)

    // Use Timer 2 in Fast PWM mode, clear on match
    TCCR2A = _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
    
    // Reset timer on TOP (OCR2A), prescaler = 1
    TCCR2B = _BV(WGM22) | _BV(CS20);
    
    // TOP (OCR2A) is modulation frequency. Timer 2 resets on this number.
    OCR2A = MOD_COUNTER_VAL;
    
    // Set the PWM value to 50%
    OCR2B = MOD_COUNTER_VAL >> 1;   // #define won't divide nicely
    
    // Enable Timer 2 overflow interrupt
    TIMSK2 = _BV(TOIE2);
    
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
#error Processor not supported
#endif

    // Disable receiver by default
    _recv_enabled = false;
 
    // Turn off IR LED initially
    pulse(false);
    
    return true;
}

/**
 * @brief Turn on the receiver
 */
void IntarIR::enableReceiver()
{   
    // Reset receiver parameters
    _recv_state = RECV_STATE_WAITING;
    _recv_tick_counter = 0;
    _recv_raw_len = 0;
    _recv_head = 0;
    _recv_tail = 0;
    
    // Enable receiver
    _recv_enabled = true;
}

/**
 * @brief Turn off the receiver
 */
void IntarIR::disableReceiver()
{
    _recv_enabled = false;
}

/**
 * @brief Transmit a packet over IR
 *
 * @param data Data to send over IR
 * @param len Length (in bytes) of data
 */
void IntarIR::xmit(byte data[], uint8_t len)
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
 * @brief Wait until the current IR transmission finished
 */
void IntarIR::flushXmit()
{
    while ( _xmit_state != XMIT_STATE_WAITING ) {
        delayMicroseconds(1);
    }
}

/**
 * @brief Transmit IR packets in a non-blocking manner
 */
void IntarIR::doXmit()
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
void IntarIR::pulse(boolean on)
{
    if ( on ) {
#if defined(__AVR_ATmega328P__)
        TCCR2A |= _BV(COM2B1);
#elif defined(KINETISL)
        *portConfigRegister(IR_LED_PIN) = PORT_PCR_MUX(4);
#endif
    } else {
#if defined(__AVR_ATmega328P__)
        TCCR2A &= ~(_BV(COM2B1));
#elif defined(KINETISL)
        *portConfigRegister(IR_LED_PIN) = PORT_PCR_MUX(1);
#endif
    }
}

/**
 * @brief Store the tick counter value in the ring buffer
 */
void IntarIR::storeCounter()
{
    //***TODO***
}

/**
 * @brief Receive IR packets asynchronously and store them in a buffer
 */
void IntarIR::doRecv()
{
    uint8_t ir;
    
    // Count our ticks every time
    _recv_tick_counter++;
    
    // Sample
    ir = (uint8_t)digitalRead(_recv_pin);
    
    // ***TODO: Handle buffer overflow?***
    
    switch( _recv_state ) {
        
        // Wait for a packet to start
        case RECV_STATE_WAITING:
            if ( ir == RECV_PULSE ) {
                _recv_raw_len = 0;
                _recv_tick_counter = 0;
                _recv_state = RECV_STATE_PULSE;
            }
            break;
            
        // Pulse state
        case RECV_STATE_PULSE:
            if ( ir == RECV_SPACE ) {
                storeCounter();
                _recv_raw_len++;
                _recv_tick_counter = 0;
                _recv_state = RECV_STATE_SPACE;
            }
            break;
        
        // Space state
        case RECV_STATE_SPACE:
            if ( _recv_tick_counter >= RECV_EOM_SPACE_TICKS ) {
                storeCounter();
                _recv_raw_len++;
                _recv_tick_counter = 0;
                _recv_state = RECV_STATE_WAITING;
            }
            if ( ir == RECV_PULSE ) {
                storeCounter();
                _recv_raw_len++;
                _recv_tick_counter = 0;
                _recv_state = RECV_STATE_PULSE;
            }
            break;
                
            
        // Unknown case
        default:
            break;
    }
}

/**
 * @brief ISR for IntarIR
 *
 * Handle transmitting and receiving IR data. Each overflow is a "tick." Each
 * lowest division of the IR protocol is a "block." 
 */
void IntarIR::isr()
{
    // Measure ticks. Every block, perform the transmit function.
    _xmit_tick_counter++;
    if ( _xmit_tick_counter >= XMIT_TICKS_PER_BLOCK ) {
        _xmit_tick_counter = 0;
        doXmit();
    }
    
    // If our receiver is enabled, perform receiver functions every tick
    if ( _recv_enabled ) {
        doRecv();
    }
}

/**
 * @brief Global interrupt service routine for timer
 *
 * We define ISR here to allow making calls to functions in the IntarIR class.
 * To do this, we instantiate a IntarIR object globally in the .cpp file.
 **/
#if defined(__AVR_ATmega328P__)
ISR(TIMER2_OVF_vect)
{
    Intar_IR.isr();
}
#elif defined(KINETISL)
void ftm0_isr(void)
{
    // Clear timer overflow flags
    FTM0_SC |= (1 << 7);
    
    // Execute Intar IR's ISR
    Intar_IR.isr();
}
#endif