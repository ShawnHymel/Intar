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

// Pin constants
#define IR_LED_PIN              6

// Transmission constants
#define MOD_FREQUENCY           38000   // 38 kHz modulation frequency
#define XMIT_BLOCK_TIME         281     // Time per block (us)
#define SOM_PULSE_BLOCKS        16       // Number of blocks in SOM pulse
#define SOM_SPACE_BLOCKS        8       // Number of blocks in SOM space
#define ZERO_PULSE_BLOCKS       1       // Number of blocks in '0' pulse
#define ZERO_SPACE_BLOCKS       1       // Number of blocks in '0' space
#define ONE_PULSE_BLOCKS        1       // Number of blocks in '1' pulse
#define ONE_SPACE_BLOCKS        3       // Number of blocks in '1' space
#define EOM_PULSE_BLOCKS        1       // Number of blocks in EOM pulse
#define EOM_SPACE_BLOCKS        4       // Number of blocks in EOM space

// Derived transmission parameters
#define MOD_COUNTER_VAL         (F_CPU / (2 * MOD_FREQUENCY)) - 1

// Debug switch
#define DEBUG_IR        0

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
    void pulse(boolean on);

private:

    // Interrupt service routing that is called by the system's ISR
    inline void isr();

};

// We need to declare a singular, global instance of our IntarIR object
extern IntarIR Intar;

#endif // IntarIR_H