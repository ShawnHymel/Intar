/****************************************************************
Receive.ino
Intar Infrared Receive Test
Shawn Hymel
June 13, 2015

Test the Intar's ability to receive data over the IR link. Send
packets from a transmitter to see the results printed in the
Serial monitor.

Hardware Connections:

 Pin 11 -> IR Receiver (38 kHz)
 Pin 13 -> LED (default on most Arduinos)
 
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

// Pins
const int receiver_pin = 11;
const int hit_pin = 13;

// Shot packet message
byte shot_packet[] = {0x10, 0xEF, 0x08, 0xF7};
uint8_t shot_packet_size = 4;

// Packet buffer
uint8_t packet[MAX_PACKET_SIZE];
uint8_t num_bytes;

void setup() {
  
  // Start serial console for debugging
  Serial.begin(9600);
  
  // Set pin modes
  pinMode(receiver_pin, INPUT);
  pinMode(hit_pin, OUTPUT);
  digitalWrite(hit_pin, LOW);

  // Initialize Intar system
  if ( Intar_Phys.begin(receiver_pin) == false ) {
    Serial.println(F("Could not start Intar IR."));
    while(1);
  }
  Serial.println(F("Receiver initialized. Don't tase me, bro!"));
  
  // Enable receiver
  Intar_Phys.enableReceiver();
}

void loop() {
  
  // Check if we have any packets and print them
  if ( Intar_Phys.available() ) {
    memset(packet, 0, MAX_PACKET_SIZE);
    num_bytes = Intar_Phys.read(packet);
    if ( num_bytes == 0 ) {
        Serial.println("PACKET EMPTY");
    } else if ( num_bytes == RECV_ERROR ) {
        Serial.println("RECV ERROR");
    } else {
        for ( int i = 0; i < num_bytes; i++ ) {
            Serial.print(packet[i], HEX);
            Serial.print(" ");
            if ( memcmp(packet, shot_packet, num_bytes) == 0 ) {
              digitalWrite(hit_pin, HIGH);
              delay(10);
              digitalWrite(hit_pin, LOW);
            }
        }
      Serial.println();
    }
  }
}