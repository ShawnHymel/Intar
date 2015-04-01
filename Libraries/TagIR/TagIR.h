/**
 * @file       TagIR.h
 * @brief 	   Library for the Arduino-based laser tag system
 * @author     Shawn Hymel
 * @copyright  2015 Shawn Hymel, Nick Poole
 * @license    http://opensource.org/licenses/MIT
 */

#ifndef TagIR_H
#define TagIR_H

#include <Arduino.h>

// Debug switch
#define DEBUG_TAGIR         0

// Interrupt service routine
ISR(TIMER2_OVF_vect);

// TagIR class
class TagIR {
    
    // The ISR is our friend! It can call our functions
    friend void TIMER2_OCV_vect();
    
public:

private:

    // Interrupt service routing that is called by the system's ISR
    inline void isr();

};

// We need to declare a singular, global instance of our TagIR object
extern TagIR Tag;

#endif // TagIR_H