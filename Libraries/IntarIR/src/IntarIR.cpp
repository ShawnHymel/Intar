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
IntarIR Intar;

/**
 * @brief Constructor
 */
IntarIR::IntarIR()
{
    
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
bool IntarIR::begin()
{
    
    // Set up the hardware PWM for the IR LED
#if defined(__AVR_ATmega328p__)

#elif defined(KINETISL)

    // Set overflow value
    FTM0_SC = 0;
    FTM0_CNT = 0;
    FTM0_MOD = MOD_COUNTER_VAL;
    
    // Enable TOIE and set prescaler to 1
    FTM0_SC = 0b011001000;
    
    // Enable PWM with set on reload and clear on match
    FTM0_C4SC = 0;
    delayMicroseconds(1);
    FTM0_C4SC = 0b11101000;
    
    // We want 50% duty cycle PWM
    FTM0_C4V = FTM0_MOD / 2;
    
    // Turn off IR LED initially
    pulse(false);

#else
    // Processor not supported
# if DEBUG_IR
    Serial.println("Processor not supported");
# endif
    return false;
#endif
    
    
    return true;
}

/**
 * @brief Turn on the IR LED with modulation
 *
 * @param on True to turn on LED, false to turn it off.
 */
void IntarIR::pulse(boolean on)
{
    if ( on ) {
#if defined(__AVR_ATmega328p__)

#elif defined(KINETISL)
        *portConfigRegister(IR_LED_PIN) = PORT_PCR_MUX(4);
#endif
    } else {
#if defined(__AVR_ATmega328p__)

#elif defined(KINETISL)
        *portConfigRegister(IR_LED_PIN) = PORT_PCR_MUX(0);
#endif
    }
}

/**
 * @brief ISR for IntarIR
 *
 * Handle transmitting and receiving IR data.
 */
void IntarIR::isr()
{
    
}

/**
 * @brief Global interrupt service routine for timer
 *
 * We define ISR here to allow making calls to functions in the IntarIR class. To
 * do this, we instantiate a IntarIR object globally in the .cpp file.
 **/
#if defined(__AVR_ATmega328P__)
ISR(TIMER2_OVF_vect)
{
    Intar.isr();
}
#elif defined(KINETISK)
void ftm0_isr(void)
{
    Intar.isr();
}
#endif