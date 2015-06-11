/**
 * @file       IntarIR.h
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

#ifndef IntarIR_H
#define IntarIR_H

#include <Arduino.h>

// Debug switch
#define DEBUG_IR                 1

// IR LED pin constant
#if defined(__AVR_ATmega328P__)
# define IR_LED_PIN              3       // Pin 3 on Arduino UNO, Pro Mini, etc.
#elif defined(KINETISL)
# define IR_LED_PIN              6       // Pin 6 on the Teensy LC
#else
#error Processor not supported
#endif

// Protocol parameters
#define MOD_FREQUENCY           38000   // 38 kHz modulation frequency
#define XMIT_BLOCK_TIME         562     // Time per block (us)
#define SOM_PULSE_BLOCKS        4       // Number of blocks in SOM pulse
#define SOM_SPACE_BLOCKS        1       // Number of blocks in SOM space
#define ZERO_PULSE_BLOCKS       1       // Number of blocks in '0' pulse
#define ZERO_SPACE_BLOCKS       1       // Number of blocks in '0' space
#define ONE_PULSE_BLOCKS        1       // Number of blocks in '1' pulse
#define ONE_SPACE_BLOCKS        2       // Number of blocks in '1' space
#define EOM_PULSE_BLOCKS        1       // Number of blocks in EOM pulse
#define EOM_SPACE_BLOCKS        2       // Number of blocks in EOM space
#define MAX_PACKET_SIZE         8       // Maximum number of bytes in a packet

// Derived protocol parameters
#define MOD_COUNTER_VAL         (F_CPU / MOD_FREQUENCY) - 1
#define XMIT_TICKS_PER_BLOCK    (XMIT_BLOCK_TIME / (1000000 / MOD_FREQUENCY))

// Transmission constants
#define XMIT_BUF_SIZE           MAX_PACKET_SIZE
#define XMIT_STATE_WAITING      0
#define XMIT_STATE_SOM_PULSE    1
#define XMIT_STATE_SOM_SPACE    2
#define XMIT_STATE_MSG          3
#define XMIT_STATE_EOM_PULSE    4
#define XMIT_STATE_EOM_SPACE    5

// Other constants
#define BITS_PER_BYTE           8

// Interrupt service routine
#if defined(__AVR_ATmega328P__)
ISR(TIMER2_OVF_vect);
#endif

// IntarIR class
class IntarIR {
    
    // The ISR is our friend! It can call our private functions
#if defined(__AVR_ATmega328P__)
    friend void TIMER2_OVF_vect();
#elif defined(KINETISL)
    friend void ftm0_isr();
#endif
 
public:
    IntarIR();
    ~IntarIR();
    bool begin();
    void xmit(byte data[], uint8_t len);

private:

    // IR transmission
    void flushXmit();
    void doXmit();
    void pulse(boolean on);

    // Interrupt service routing that is called by the system's ISR
    inline void isr();

    // Members for transmission
    volatile uint8_t _xmit_tick_counter;
    volatile int8_t _xmit_block_counter;
    volatile uint8_t _xmit_state;
    volatile boolean _bit_pulse;
    volatile uint8_t _xmit_bit;
    volatile uint8_t _xmit_byte;
    volatile uint8_t _bytes_to_send;
    volatile int8_t _xmit_ptr;
    byte _xmit_buf[XMIT_BUF_SIZE];
};

// We need to declare a singular, global instance of our IntarIR object
extern IntarIR Intar_IR;

#endif // IntarIR_H