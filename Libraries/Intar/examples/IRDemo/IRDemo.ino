/****************************************************************
IRDemo.ino
Intar Infrared Demo
Shawn Hymel
July 15, 2015

Demo sketch that sends and receives packets. Play a mini-tag
game. 5 HP, semi-auto, 1 damage shots. Health recharges after
5 seconds. Death at 0 HP, firing disabled. Respawn in 10
seconds.

Hardware Connections (Teensy):

 Pin 2 -> Trigger
 Pin 6 -> IR LED (hard-coded)
 Pin 9 -> Status LED (PWM)
 Pin 11 -> IR Receiver (38 kHz)
 
Hardware Connections (328p):

 Pin 2 -> Trigger
 Pin 3 -> IR LED (hard-coded)
 Pin 9 -> Status LED (PWM)
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
const int trigger_pin = 2;
const int status_pin = 9;
const int receiver_pin = 11;

// Button states (for debouncing the trigger)
uint8_t button_sample = HIGH;
uint8_t button_state = HIGH;
uint8_t prev_button_state = HIGH;
long prev_debounce_time;
const long debounce_delay = 30; // ms

// Global Intar object
IntarIR intar_ir(receiver_pin);

// Shot packet message
byte shot_packet[] = {0x10, 0xEF, 0x08, 0xF7};
uint8_t shot_packet_size = 4;

// Packet buffer
uint8_t packet[MAX_PACKET_SIZE];
uint8_t num_bytes;

void setup() {
  
  // Start serial console for debugging
  Serial.begin(9600);
    
  // Initialize pins
  pinMode(trigger_pin, INPUT_PULLUP);
  pinMode(status_pin, OUTPUT);
  analogWrite(status_pin, 10);
  
  // Enable transmitter and receiver
  intar_ir.enableTransmitter();
  intar_ir.enableReceiver();
  
  // Let the user know we can begin
  Serial.println("Initialized. There can only be one!");
}

void loop() {
  
  Serial.println(Intar_Phys.testvar);
  
  // Check if we have any packets and print them
  if ( intar_ir.available() ) {
    num_bytes = intar_ir.read(packet);
    if ( num_bytes == 0 ) {
      Serial.println("PACKET EMPTY");
    } else if ( num_bytes == RECV_ERROR ) {
      Serial.println("RECV ERROR");
    } else {
      for ( int i = 0; i < num_bytes; i++ ) {
        Serial.print(packet[i], HEX);
        Serial.print(" ");
        if ( memcmp(packet, shot_packet, num_bytes) == 0 ) {
          analogWrite(status_pin, 200);
          delay(10);
          analogWrite(status_pin, 10);
        }
      }
      Serial.println();
    }
  } 
}