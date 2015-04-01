/**
 * @file       TagIR.cpp
 * @brief 	   Library for the Arduino-based laser tag system
 * @author     Shawn Hymel
 * @copyright  2015 Shawn Hymel, Nick Poole
 * @license    http://opensource.org/licenses/MIT
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
ISR(TIMER2_OVF_vect)
{
    Tag.isr();
}