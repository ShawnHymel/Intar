/****************************************************************
IntarIR_Transmit.ino
Intar Infrared Transmit Test
Shawn Hymel
May 31, 2015

Test the Intar's ability to send data over the IR link. Press
the attached button to send shot packets. Note that pin 6 is
hardcoded in the IntarIR library. This is because it is attached
to a specific timer (FTM0).

Hardware Connections:

 Teensy LC Pin 6 -> IR LED
 Teensy LC Pin 2 -> Button
 
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
const int trigger_pin = 2;

// Shot packet message
byte shot_packet[] = {0x10, 0xEF, 0x08, 0xF7};
uint8_t shot_packet_size = 4;

void setup() {
  
  // Set up GPIO
  pinMode(trigger_pin, INPUT_PULLUP);
  
  // Start serial console for debugging
  Serial.begin(9600);

  // Initialize Intar system
  if ( Intar_IR.begin() == false ) {
    Serial.println(F("Could not start Intar IR."));
  }
}

void loop() {
  
  // If the button is pushed, fire IR shot packets
  if ( digitalRead(trigger_pin) == 0 ) {
    Intar_IR.xmit(shot_packet, shot_packet_size);
    delay(1);
  }
}