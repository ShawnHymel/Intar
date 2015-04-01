/**
 * @file       TagIRInt.h
 * @brief 	   Library for the Arduino-based laser tag system
 * @author     Shawn Hymel
 * @copyright  2015 Shawn Hymel, Nick Poole
 * @license    http://opensource.org/licenses/MIT
 *
 * Interrupt parameters
 */
 
#ifndef TagIRInt_H
#define TagIRInt_H

// Define which timer to use

// Arduino UNO, Pro Mini, etc.
#if defined __AVR_ATmega328P__
    #define IR_TIMER2
#endif

// Set the CPU clock if not defined
#ifndef F_CPU
    #define F_CPU 16 16000000
#endif

// Set up parameters for Timer2
#if defined IR_TIMER2
#endif

#endif // TagIRInt_H