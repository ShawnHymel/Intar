/**
 * @file       TagIR.cpp
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
 
#include "TagIR.h"

// We need to create a global instance so that the ISR knows what to talk to
TagIR Tag;

/**
 * @brief Constructor
 */
TagIR::TagIR()
{
    
}

/**
 * @brief Destructor
 */
TagIR::~TagIR()
{
    
}

/**
 * @brief Configures the parameters for the TagIR object. Call this first.
 *
 * @param tx_pin Transmit pin to use.
 * @param rx_pin Receive pin to use.
 * @return True on initialization success. False on failure.
 */
bool TagIR::begin(uint8_t tx_pin, uint8_t rx_pin)
{
    return true;
}

/**
 * @brief ISR for TagIR
 *
 * Handle transmitting and receiving IR data.
 */
void TagIR::isr()
{
    
}

/**
 * @brief Global interrupt service routine for timer
 *
 * We define ISR here to allow making calls to functions in the TagIR class. To
 * do this, we instantiate a TagIR object globally in the .cpp file.
 **/
#if defined(__AVR_ATmega328P__)
ISR(TIMER2_OVF_vect)
{
    Tag.isr();
}
#elif defined(KINETISK)
void ftm0_isr(void)
{
    Tag.isr();
}
#endif