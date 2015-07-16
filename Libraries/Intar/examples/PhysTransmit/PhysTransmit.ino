/****************************************************************
PhysTransmit.ino
Intar Infrared Transmit Test
Shawn Hymel
May 31, 2015

Test the Intar's ability to send data over the IR link. Press
the attached button to send shot packets. Note that the IR LED
is hardcoded in the IntarIR library. This is because it is 
attached to a specific timer (e.g. FTM0).

Hardware Connections (Teensy):

 Teensy LC Pin 6 -> IR LED
 Teensy LC Pin 2 -> Button
 
Hardware Connections (328p):

 Arduino Pin 3 -> IR LED
 Arduino Pin 2 -> Button
 
Resources:
Include IntarPhys.h

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

#include <IntarPhys.h>

// Constants
#define SEMI_AUTO       0
#define FULL_AUTO       1

// Pins
const int trigger_pin = 2;

// Shot packet message
byte shot_packet[] = {0x10, 0xEF, 0x08, 0xF7};
uint8_t shot_packet_size = 4;

// Button states (mostly needed for debouncing the trigger)
uint8_t button_sample = HIGH;
uint8_t button_state = HIGH;
uint8_t prev_button_state = HIGH;
long prev_debounce_time;
const long debounce_delay = 30;  // ms

// Firing modes
const uint8_t firing_mode = SEMI_AUTO;

void setup() {
  
  // Set up GPIO
  pinMode(trigger_pin, INPUT_PULLUP);
  
  // Start serial console for debugging
  Serial.begin(9600);

  // Initialize Intar system
  if ( Intar_Phys.begin() == false ) {
    Serial.println(F("Could not start Intar IR."));
    while(1);
  }
  
  // Enable transmitter
  Intar_Phys.enableTransmitter();
  Serial.println(F("Transmitter initialized. Fire away!"));
}

void loop() {
  
  // If the button is pushed, fire IR shot packets
  switch(firing_mode) {
  
    // Epic debouncing to prevent multi-shot
    case SEMI_AUTO:
      button_sample = digitalRead(trigger_pin);
      if ( button_sample != prev_button_state ) {
        prev_debounce_time = millis();
      }
      if ( (millis() - prev_debounce_time) >= debounce_delay ) {
        if ( button_sample != button_state ) {
          button_state = button_sample;
          if ( button_state == LOW ) {
            Intar_Phys.xmit(shot_packet, shot_packet_size);
            Serial.println("pew");
          }
        }
      }
      prev_button_state = button_sample;
      break;
    
    // As long as the trigger is held, spray away!
    case FULL_AUTO:
      if ( digitalRead(trigger_pin) == LOW ) {
        Intar_Phys.xmit(shot_packet, shot_packet_size);
        Serial.println("pew");
      }
      break;
      
    default:
      break;
  }
}