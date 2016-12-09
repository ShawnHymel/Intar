/****************************************************************
PhysReceive.ino
Intar Infrared Receive Test
Shawn Hymel
June 13, 2015
Updated: December 9, 2016

Test the Intar's ability to receive data over the IR link. Send
packets from a transmitter to see the results printed in the
Serial monitor.

Hardware Connections (328p):

 Pin 11 -> IR Receiver (38 kHz)
 Pin 13 -> LED (default on most Arduinos)

Hardware Connections (ATtiny84a):

 Arduino Pin 3 (PA3) -> IR Receiver (38 kHz)
 Arduino Pin 7 (PA7) -> LED
 
Resources:
Include IntarPhys.h

Development environment specifics:
Written in Arduino 1.6.3
Tested with a Teensy, Pro Mini 5V

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
#define DEBUG           1

#if DEBUG
# if defined(__AVR_ATtiny84__)
#  include <SoftwareSerial.h>
SoftwareSerial *dserial = new SoftwareSerial(2, 1); // Rx, Tx
# else
HardwareSerial *dserial = &Serial;
# endif
#endif

// Pins
#if defined(__AVR_ATmega328P__)
  const int receiver_pin = 11;
  const int hit_pin = 13;
#elif defined(__AVR_ATtiny84__)
  const int receiver_pin = 3;
  const int hit_pin = 7;
#elif defined(KINETISL)
  const int receiver_pin = 11;
  const int hit_pin = 13;
#else
# error Processor not supported
#endif


// Shot packet message
byte shot_packet[] = {0x10, 0xEF, 0x08, 0xF7};
uint8_t shot_packet_size = 4;

// Packet buffer
uint8_t packet[MAX_PACKET_SIZE];
uint8_t num_bytes;

void setup() {
  
  // Start serial console for debugging
#if DEBUG
  dserial->begin(9600);
  dserial->println(F("Starting Intar..."));
  dserial->print(F("F_CPU = "));
  dserial->println(F_CPU);
#endif
  
  // Set pin modes
  pinMode(receiver_pin, INPUT);
  pinMode(hit_pin, OUTPUT);
  digitalWrite(hit_pin, LOW);

  // Initialize Intar system
  if ( Intar_Phys.begin(receiver_pin) == false ) {
#if DEBUG
    dserial->println(F("Could not start Intar IR."));
#endif
    while(1);
  }
#if DEBUG
  dserial->println(F("Receiver initialized. Don't tase me, bro!"));
#endif
  
  // Enable receiver
  Intar_Phys.enableReceiver();
}

void loop() {
  
  // Check if we have any packets and print them
  if ( Intar_Phys.available() ) {
    memset(packet, 0, MAX_PACKET_SIZE);
    num_bytes = Intar_Phys.read(packet);
    if ( num_bytes == 0 ) {
#if DEBUG
        dserial->println("PACKET EMPTY");
#endif
    } else if ( num_bytes == RECV_ERROR ) {
#if DEBUG
        dserial->println("RECV ERROR");
#endif
    } else {
        for ( int i = 0; i < num_bytes; i++ ) {
#if DEBUG
            dserial->print(packet[i], HEX);
            dserial->print(" ");
#endif
            if ( memcmp(packet, shot_packet, num_bytes) == 0 ) {
              digitalWrite(hit_pin, HIGH);
              delay(10);
              digitalWrite(hit_pin, LOW);
            }
        }
#if DEBUG
      dserial->println();
#endif
    }
  }
}