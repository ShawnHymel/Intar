/****************************************************************
IRDemo.ino
Intar Infrared Demo
Shawn Hymel
July 15, 2015
Updated: December 14, 2016

Demo sketch that sends and receives packets. Play a mini-tag
game. 25 HP, semi-auto, 5 damage shots. Health recharges after
5 seconds. Death at 0 HP, firing disabled. Respawn in 10
seconds.
 
Hardware Connections (328p):

 Pin 2 -> Trigger
 Pin 3 -> IR LED (hard-coded)
 Pin 9 -> Status LED (PWM)
 Pin 11 -> IR Receiver (38 kHz)

Hardware Connections (ATtiny84a):

 Arduino Pin 6 (PA6) -> Trigger
 Arduino Pin 5 (PA5) -> IR LED (hard-coded)
 Arduino Pin 7 (PA7) -> Status LED (PWM)
 Arduino Pin 3 (PA3) -> IR Receiver (38 kHz)

Hardware Connections (Teensy LC):

 Pin 2 -> Trigger
 Pin 6 -> IR LED (hard-coded)
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

// Constants
#define DEBUG           0 // 1 for debugging over Serial
#define SHOOT_YOURSELF  0 // 1 to allow xmit and rcv at same time

#if DEBUG
# if defined(__AVR_ATtiny84__)
#  include <SoftwareSerial.h>
SoftwareSerial *dserial = new SoftwareSerial(2, 1); // Rx, Tx
# else
HardwareSerial *dserial = &Serial;
# endif
#endif
                   
// Game constants
const int SEMI_AUTO = 0;          // One trigger, one shot
const int THREE_ROUND_BURST = 1;  // One trigger, three shots
const int FULL_AUTO = 2;          // One trigger, many shots
const int SHOT_PACKET_SIZE = 4;   // Num bytes in a shot packet
const int HEADER_SHOT = 1;        // Packet header byte

// Game parameters
const int PLAYER_ID = 0;          // Null player (free-for-all)
const int TEAM_ID = 0;            // Null team (hit and be hit)
const int DAMAGE_ELEMENT = 0;     // No elemental damage
const int DAMAGE_AMOUNT = 5;      // Points of damage to inflict
const int HP_MAX = 25;            // Max player hit points
const int TIME_RECHARGE = 5;      // Seconds to recharge HP
const int TIME_RESPAWN = 10;      // Seconds to respawn
const int FIRE_MODE = SEMI_AUTO;  // How the trigger works

// Pins
#if defined(__AVR_ATmega328P__)
  const int TRIGGER_PIN = 2;        // Push this to shoot
  const int STATUS_PIN = 9;         // Shows current HP (brightness)
  const int RECEIVER_PIN = 11;      // Aim for this
#elif defined(__AVR_ATtiny84__)
  const int TRIGGER_PIN = 6;        // Push this to shoot
  const int STATUS_PIN = 7;         // Shows current HP (brightness)
  const int RECEIVER_PIN = 3;       // Aim for this
#elif defined(KINETISL)
  const int TRIGGER_PIN = 2;        // Push this to shoot
  const int STATUS_PIN = 9;         // Shows current HP (brightness)
  const int RECEIVER_PIN = 11;      // Aim for this
#else
# error Processor not supported
#endif


// Button states (for debouncing the trigger)
uint8_t button_sample = HIGH;
uint8_t button_state = HIGH;
uint8_t prev_button_state = HIGH;
long prev_debounce_time;
const long debounce_delay = 30; // ms

// Global variables
IntarIR intar_ir;
uint8_t shot_packet[SHOT_PACKET_SIZE];
uint8_t num_bytes;
uint8_t damage;
uint8_t led;
unsigned long time_since_hit;
bool recharge_flag;
int hp;

void setup() {
  
  // Start serial console for debugging
#if DEBUG
  dserial->begin(9600);
#endif
    
  // Initialize pins
  pinMode(TRIGGER_PIN, INPUT_PULLUP);
  pinMode(STATUS_PIN, OUTPUT);
  pinMode(RECEIVER_PIN, INPUT);
  
  // Permanently add header, player, and team to shot packet
  shot_packet[0] = HEADER_SHOT;   // Header = shot packet
  shot_packet[1] = PLAYER_ID;     // Player ID
  shot_packet[2] = TEAM_ID << 4;  // Team is high nybble
  
  // Set initial HP and LED
  hp = HP_MAX;
  led = 255;
  
  // Initialize IR communications
  intar_ir.begin(RECEIVER_PIN);
  intar_ir.enableTransmitter();
  intar_ir.enableReceiver();
  
  // Set our initial time since hit
  time_since_hit = millis();
  recharge_flag = false;
  
  // Let the user know we can begin
#if DEBUG
  dserial->println("Initialized. There can only be one!");
#endif
}

void loop() {
  
  // Check for and decode packets
  damage = checkForDamage();
  
  // Update HP
  hp = hp - damage;
  if ( hp < 0 ) {
    hp = 0;
  }
  if ( damage > 0 ) {
    time_since_hit = millis();
    recharge_flag = true;
#if DEBUG
    dserial->print("HP: ");
    dserial->println(hp);
#endif
  }
  
  // Update status LED
  led = map(hp, 0, HP_MAX, 8, 0);
  led = 0x80 >> led;
  analogWrite(STATUS_PIN, led);
  
  // Check if we are dead
  if ( hp <= 0 ) {
    intar_ir.disableTransmitter();
    intar_ir.disableReceiver();
#if DEBUG
    dserial->print("You are dead. Respawning in: ");
#endif
    for ( uint8_t i = 0; i < TIME_RESPAWN; i++ ) {
#if DEBUG
      dserial->print(TIME_RESPAWN - i);
      dserial->print(" ");
#endif
      delay(1000);
    }
#if DEBUG
    dserial->println("Go!");
#endif
    hp = HP_MAX;
    recharge_flag = false;
    intar_ir.enableTransmitter();
    intar_ir.enableReceiver();
  }
  
  // Check if enough time has passed to recharge HP
  if ( recharge_flag && 
       (millis() - time_since_hit >= (TIME_RECHARGE * 1000)) ) {
    hp = HP_MAX;
    recharge_flag = false;
#if DEBUG
    dserial->print("Recharged! HP: ");
    dserial->println(hp);
#endif
  }
  
  // Fire packets
  handleTrigger(FIRE_MODE);
  
}

uint8_t checkForDamage() {
  
  uint8_t recv_packet[MAX_PACKET_SIZE];
  
  // Check if we have any packets (and no errors)
  if ( intar_ir.available() ) {
    num_bytes = intar_ir.read(recv_packet);
    if ( (num_bytes == 0) || (num_bytes == RECV_ERROR) ) {
      return 0;
    }
  } else {
    return 0;
  }
  
  // Make sure header is 0x01 (for shot packet)
  if ( recv_packet[0] != HEADER_SHOT ) {
    return 0;
  }
  
  // Extract damage value (we can igore player, team, element)
  return recv_packet[3];
}

void handleTrigger(uint8_t fire_mode) {
  
  switch(fire_mode) {
    
    // Fire one packet per trigger pull
    case SEMI_AUTO:
      button_sample = digitalRead(TRIGGER_PIN);
      if ( button_sample != prev_button_state ) {
        prev_debounce_time = millis();
      }
      if ( (millis() - prev_debounce_time) >= debounce_delay ) {
        if ( button_sample != button_state ) {
          button_state = button_sample;
          if ( button_state == LOW ) {
            fire(DAMAGE_AMOUNT, DAMAGE_ELEMENT);
#if DEBUG
            dserial->println("pew");
#endif
          }
        }
      }
      prev_button_state = button_sample;
      break;
      
    // Fire three packets per trigger pull
    case THREE_ROUND_BURST:
      button_sample = digitalRead(TRIGGER_PIN);
      if ( button_sample != prev_button_state ) {
        prev_debounce_time = millis();
      }
      if ( (millis() - prev_debounce_time) >= debounce_delay ) {
        if ( button_sample != button_state ) {
          button_state = button_sample;
          if ( button_state == LOW ) {
            for ( uint8_t i = 0; i < 3; i++ ) {
              fire(DAMAGE_AMOUNT, DAMAGE_ELEMENT);
#if DEBUG
              dserial->println("pew");
#endif
            }
          }
        }
      }
      prev_button_state = button_sample;
      break;
      
    // Spray and pray!
    case FULL_AUTO:
      if ( digitalRead(TRIGGER_PIN) == LOW ) {
        fire(DAMAGE_AMOUNT, DAMAGE_ELEMENT);
#if DEBUG
        dserial->println("pew");
#endif
      }
      break;
      
    // Unknown fire mode
    default:
      break;
  }
}

void fire(uint8_t damage, uint8_t element) {
  
  // Set  element type
  shot_packet[2] &= 0xF0;               // Clear low nybble
  shot_packet[2] |= (0x0F & element);   // Low nybble is element
  
  // Set damage
  shot_packet[3] = damage;              // Last byte is damage
  
  // Fire!
#if !SHOOT_YOURSELF
  intar_ir.disableReceiver();
#endif
  intar_ir.send(shot_packet, SHOT_PACKET_SIZE);
#if !SHOOT_YOURSELF
  intar_ir.flushTransmitter();
  intar_ir.enableReceiver();
#endif
}