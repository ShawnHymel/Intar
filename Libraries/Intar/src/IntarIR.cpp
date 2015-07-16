/**
 * @file       IntarIR.cpp
 * @brief      Library for the IR data link layer in Arduino-based laser tag
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

/***
 * @brief Set up IR communications and assign receiver pin.
 *
 * @param[in] receiver_pin pin connected to IR receiver
 * @return True on initialization success. False on failure.
 */
bool IntarIR::begin(uint8_t receiver_pin /* = 0 */)
{
    return Intar_Phys.begin(receiver_pin);
}

/**
 * @brief Turn on the IR LED transmitter
 */
void IntarIR::enableTransmitter()
{
   Intar_Phys.enableTransmitter();
}

/**
 * @brief Turn off the transmitter
 */
void IntarIR::disableTransmitter()
{
    Intar_Phys.disableTransmitter();
}

/**
 * @brief Turn on the receiver
 */
void IntarIR::enableReceiver()
{   
    Intar_Phys.enableReceiver();
}

/**
 * @brief Turn off the receiver
 */
void IntarIR::disableReceiver()
{
    Intar_Phys.disableReceiver();
}

/**
 * @brief Determines if there is data in the receive buffer ready to be read
 *
 * @return number of packets waiting to be read
 */
uint8_t IntarIR::available()
{
    return Intar_Phys.available();
}

/**
 * @brief Read the next packet from the ring buffer
 *
 * @param[out] packet pointer to buffer to store the read packet
 * @return RECV_ERROR (-1) on error. Number of bytes in packet otherwise.
 */
uint8_t IntarIR::read(uint8_t packet[MAX_PACKET_SIZE])
{
    return Intar_Phys.read(packet);
}