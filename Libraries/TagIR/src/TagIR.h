/**
 * @file       TagIR.h
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

#ifndef TagIR_H
#define TagIR_H

#include <Arduino.h>

// Debug switch
#define DEBUG_TAGIR         0

// Interrupt service routine
#if defined(__AVR_ATmega328P__)
ISR(TIMER2_OVF_vect);
#endif

// TagIR class
class TagIR {
    
    // The ISR is our friend! It can call our private functions
#if defined(__AVR_ATmega328P__)
    friend void TIMER2_OVF_vect();
#elif defined(KINETISK)
    friend void ftm0_isr();
#endif
 
public:
    TagIR();
    ~TagIR();
    bool begin(uint8_t tx_pin, uint8_t rx_pin);

private:

    // Interrupt service routing that is called by the system's ISR
    inline void isr();

};

// We need to declare a singular, global instance of our TagIR object
extern TagIR Tag;

#endif // TagIR_H