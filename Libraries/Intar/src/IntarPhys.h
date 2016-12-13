/**
 * @file       IntarPhys.h
 * @brief 	   Library for the IR physical layer in Arduino-based laser tag
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

#ifndef IntarPhys_H
#define IntarPhys_H

#include <Arduino.h>

// Debug switch
#define DEBUG_PHYS               0

// IR LED pin constant
#if defined(__AVR_ATmega328P__)
# define IR_LED_PIN              3       // Pin 3 on Arduino UNO, Pro Mini, etc.
#elif defined(__AVR_ATtiny84__)
# define IR_LED_PIN              5       // PA5 (OC1B) on the ATtiny84/84a
#elif defined(KINETISL)
# define IR_LED_PIN              6       // Pin 6 on the Teensy LC
#else
# error Processor not supported
#endif

// Protocol parameters
#define MOD_FREQUENCY           38000   // 38 kHz modulation frequency
#define XMIT_BLOCK_TIME         562     // Time per block (us)
#define SOM_PULSE_BLOCKS        2       // Number of blocks in SOM pulse
#define SOM_SPACE_BLOCKS        1       // Number of blocks in SOM space
#define BIT_PULSE_BLOCKS        1       // Number of blocks in SOM bit
#define ZERO_SPACE_BLOCKS       1       // Number of blocks in '0' space
#define ONE_SPACE_BLOCKS        2       // Number of blocks in '1' space
#define EOM_PULSE_BLOCKS        1       // Number of blocks in EOM pulse
#define EOM_SPACE_BLOCKS        3       // Number of blocks in EOM space
#define MAX_PACKET_SIZE         8       // Maximum number of bytes in a packet

// Derived protocol parameters
#define ZERO_PULSE_BLOCKS       BIT_PULSE_BLOCKS    // Blocks in '0' pulse
#define ONE_PULSE_BLOCKS        BIT_PULSE_BLOCKS    // Blocks in '1' pulse
#define MOD_COUNTER_VAL         (F_CPU / MOD_FREQUENCY) - 1
#define TICK_TIME               (1000000 / MOD_FREQUENCY) // Time per tick (us)
#define XMIT_TICKS_PER_BLOCK    (XMIT_BLOCK_TIME / (1000000 / MOD_FREQUENCY))

// Transmission constants
#define XMIT_BUF_SIZE           MAX_PACKET_SIZE
#define XMIT_STATE_WAITING      0
#define XMIT_STATE_SOM_PULSE    1
#define XMIT_STATE_SOM_SPACE    2
#define XMIT_STATE_MSG          3
#define XMIT_STATE_EOM_PULSE    4
#define XMIT_STATE_EOM_SPACE    5

// Receiver constants
#define RECV_PULSE              0       // Most IR receivers are active low
#define RECV_SPACE              1       // Most IR receivers idle high
#define RECV_TICKS_PER_SAMPLE	5		// Number of ticks between samples
#define RECV_SAMPLE_MAX			200		// Maximum number of samples to record
#define RECV_MAX_PACKETS        8       // Size of ring buffer (num packets)
#define RECV_STATE_IDLE         0
#define RECV_STATE_SOM_PULSE    1
#define RECV_STATE_SOM_SPACE    2
#define RECV_STATE_BIT_PULSE    3
#define RECV_STATE_BIT_SPACE    4
#define RECV_ERROR              0xFF

// Receiver: samples per block (min and max)
#define RECV_SOM_PULSE_MIN     ((SOM_PULSE_BLOCKS * XMIT_BLOCK_TIME) / \
                                (RECV_TICKS_PER_SAMPLE * TICK_TIME)) - 1
#define RECV_SOM_PULSE_MAX     ((SOM_PULSE_BLOCKS * XMIT_BLOCK_TIME) / \
                                (RECV_TICKS_PER_SAMPLE * TICK_TIME)) + 2
#define RECV_SOM_SPACE_MIN     ((SOM_SPACE_BLOCKS * XMIT_BLOCK_TIME) / \
                                (RECV_TICKS_PER_SAMPLE * TICK_TIME)) - 1
#define RECV_SOM_SPACE_MAX     ((SOM_SPACE_BLOCKS * XMIT_BLOCK_TIME) / \
                                (RECV_TICKS_PER_SAMPLE * TICK_TIME)) + 2
#define RECV_BIT_PULSE_MIN     ((BIT_PULSE_BLOCKS * XMIT_BLOCK_TIME) / \
                                (RECV_TICKS_PER_SAMPLE * TICK_TIME)) - 1
#define RECV_BIT_PULSE_MAX     ((BIT_PULSE_BLOCKS * XMIT_BLOCK_TIME) / \
                                (RECV_TICKS_PER_SAMPLE * TICK_TIME)) + 2
#define RECV_ZERO_SPACE_MIN    ((ZERO_SPACE_BLOCKS * XMIT_BLOCK_TIME) / \
                                (RECV_TICKS_PER_SAMPLE * TICK_TIME)) - 1
#define RECV_ZERO_SPACE_MAX    ((ZERO_SPACE_BLOCKS * XMIT_BLOCK_TIME) / \
                                (RECV_TICKS_PER_SAMPLE * TICK_TIME)) + 2
#define RECV_ONE_SPACE_MIN     ((ONE_SPACE_BLOCKS * XMIT_BLOCK_TIME) / \
                                (RECV_TICKS_PER_SAMPLE * TICK_TIME)) - 1
#define RECV_ONE_SPACE_MAX     ((ONE_SPACE_BLOCKS * XMIT_BLOCK_TIME) / \
                                (RECV_TICKS_PER_SAMPLE * TICK_TIME)) + 2
#define RECV_IDLE_SPACE_MIN    (((ONE_SPACE_BLOCKS + 1) * XMIT_BLOCK_TIME) / \
                                (RECV_TICKS_PER_SAMPLE * TICK_TIME)) - 1

// Other constants
#define BITS_PER_BYTE           8

// Interrupt service routine
#if defined(__AVR_ATmega328P__)
ISR(TIMER2_OVF_vect);
#elif defined(__AVR_ATtiny84__)
ISR(TIM1_OVF_vect);
#endif

// IntarIR class
class IntarPhys {
    
    // The ISR is our friend! It can call our private functions
#if defined(__AVR_ATmega328P__)
    friend void TIMER2_OVF_vect();
#elif defined(__AVR_ATtiny84__)
    friend void TIM1_OVF_vect();
#elif defined(KINETISL)
    friend void ftm0_isr();
#endif
 
public:
    IntarPhys();
    ~IntarPhys();
    bool begin(uint8_t recv_pin = 0);
    void enableTransmitter();
    void disableTransmitter();
    void enableReceiver();
    void disableReceiver();
    void xmit(uint8_t data[], uint8_t len);
    uint8_t available();
    bool overflow();
    uint8_t read(uint8_t packet[MAX_PACKET_SIZE]);
    
    //***SRH***
    int dstate;
    unsigned long utime;
	
private:

    // IR transmission
    void flushXmit();
    void doXmit();
    void pulse(bool on);
    
    // IR receive
    void doRecv();
    void storeBit(uint8_t recv_bit);
    void storePacket();
    void handleRecvError();

    // Interrupt service routing that is called by the system's ISR
    inline void isr();

    // Members for transmitter
    volatile bool _xmit_enabled;
    volatile uint8_t _xmit_tick_counter;
    volatile int8_t _xmit_block_counter;
    volatile uint8_t _xmit_state;
    volatile bool _bit_pulse;
    volatile uint8_t _xmit_bit;
    volatile uint8_t _xmit_byte;
    volatile uint8_t _bytes_to_send;
    volatile int8_t _xmit_ptr;
    byte _xmit_buf[XMIT_BUF_SIZE];
    
    // Members for receiver
    uint8_t _recv_pin;
    volatile bool _recv_enabled;
    volatile uint16_t _recv_tick_counter;
	volatile uint16_t _recv_sample_counter;
    volatile uint8_t _recv_state;
    volatile uint8_t _recv_bit_ptr;
    volatile uint8_t _recv_byte_ptr;
    uint8_t _recv_bytes[RECV_MAX_PACKETS];
	uint8_t _recv_buffer[MAX_PACKET_SIZE * RECV_MAX_PACKETS];
    volatile uint8_t _recv_head;
    volatile uint8_t _recv_tail;
    volatile bool _recv_ring_overflow;
};

// We need to declare a singular, global instance of our IntarIR object
extern IntarPhys Intar_Phys;

#endif // IntarPhys_H