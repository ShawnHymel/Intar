/**
 * @file       IntarIR.h
 * @brief 	   Library for the IR data link layer in Arduino-based laser tag
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
#include <IntarPhys.h>

// Debug switch
#define DEBUG_IR               0

// IntarIR class
class IntarIR {
    
public:
    IntarIR();
    ~IntarIR();
    bool begin(uint8_t receiver_pin = 0);
    void enableTransmitter();
    void disableTransmitter();
    void enableReceiver();
    void disableReceiver();
    uint8_t available();
    uint8_t read(uint8_t packet[MAX_PACKET_SIZE]);
    
private:

};

#endif // IntarIR_H