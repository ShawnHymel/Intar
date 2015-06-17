/****************************************************************
IntarIR_Receive.ino
Intar Infrared Receive Test
Shawn Hymel
June 13, 2015

Test the Intar's ability to receive data over the IR link. Send
packets from a transmitter to see the results printed in the
Serial monitor.

Hardware Connections:

 Pin 11 -> IR Receiver (38 kHz)
 
Resources:
Include IntarIR.h

Development environment specifics:
Written in Arduino 1.6.3
Tested with a Teensy 

License: http://opensource.org/licenses/MIT

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************/

#include <IntarIR.h>

// Pins
const int receiver_pin = 11;

// Shot packet message
byte shot_packet[] = {0x10, 0xEF, 0x08, 0xF7};
uint8_t shot_packet_size = 4;

void setup() {
  
  // Start serial console for debugging
  Serial.begin(9600);

  // Initialize Intar system
  if ( Intar_IR.begin(receiver_pin) == false ) {
    Serial.println(F("Could not start Intar IR."));
  }
  
  // Enable receiver
  Intar_IR.enableReceiver();
}

void loop() {

}