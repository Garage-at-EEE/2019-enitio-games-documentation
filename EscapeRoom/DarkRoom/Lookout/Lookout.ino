// Last modified: 8 August 2019
// Approximate ADC reading level of obstacle sensor + 10k resistor in a dark room: 4.
// Use 200-270R for IR LED, 10k for IR sensor.

#include <RedMP3.h>
#include <SPI.h>
#include <RF24.h>

// DEBUG FLAG. UNCOMMENT TO DEBUG (USE SERIAL PRINT).
//#define DEBUG

// Hardware SPI pins. Cann't be changed.
// Pin 13: SCK
// Pin 12: MISO
// Pin 11: MOSI

// Pin configurations.
#define IR_SENSE_1  A0
#define IR_SENSE_2  A1
#define RF_CE       9
#define RF_CS       10
#define MP3_TX      7
#define MP3_RX      8

// Other configurations.
#define VOLUME       30               // Volume of MP3 shield. Max is 30.
#define AUDIO_LENGTH 2300             // Scream interval in ms.
#define THRESHOLD    100              // Set analog threshold for IR sensor.
#define RF_CHANNEL   100              // Set RF24 channel.
byte address[][6] = {"hcm", "hcm"};   // Address for human circuit RF24s {WRITE to, READ from}.

// Characters (agreed on) to send to / receive from Human Circuit Game.
char eye_covered = 'z';      // Send to HC.
char eye_uncovered = 'y';    // Send to HC.
char paint_touched = '3';    // Receive from HC.
char paint_released = 'c';   // Receive from HC.

// Instantiate objects.
RF24 rf(RF_CE, RF_CS);
MP3 mp3(MP3_RX, MP3_TX);

unsigned long last_scream;            // Used for recording last scream millis().
boolean is_eye_covered = 0;           // Flag: eye is covered.
boolean is_eye_covered_prev = 0;      // Flag: hcm was informed eye is covered.
char received_char = paint_released;  // To store received character.


void setup() {

  pinMode(LED_BUILTIN, OUTPUT);    // For indicator.
  pinMode(IR_SENSE_1, INPUT);
  pinMode(IR_SENSE_2, INPUT);

  // MP3 module setup.
  mp3.begin();
  mp3.pause();      // NEED THIS INSTRUCTION!!!
  delay(10);
  mp3.setVolume(VOLUME);

  // RF24 setup.
  rf.begin();
  rf.setPALevel(RF24_PA_MAX);
  rf.setDataRate(RF24_250KBPS);
  rf.setChannel(RF_CHANNEL);
  rf.openWritingPipe(address[0]);      // Write to address[0].
  rf.openReadingPipe(1, address[1]);   // Read from address[1].
  rf.startListening();

  #ifdef DEBUG
  Serial.begin(9600);
  #endif
}


//------------------- Game Starts -------------------//

// As long as the eye is not covered, the skeleton screams if the painting is touched.
// Eye covered: analogRead() HIGHER than the threshold.
// If the eye is covered, the eye_covered_send character will be sent repeatedly until the HCM acknowledges.

void loop() {

  is_eye_covered = ((analogRead(IR_SENSE_1) >= THRESHOLD) && (analogRead(IR_SENSE_2) >= THRESHOLD));    // Update eye flag.

  if (rf.available())
  {
    rf.read( &received_char, sizeof(char) );     // Update received_char (if available).
    indicateBlink();

    #ifdef DEBUG
    Serial.print(F("Received: "));
    Serial.println(received_char);
    #endif
  }

  if ( !is_eye_covered && (received_char == paint_touched) && (millis() - last_scream > AUDIO_LENGTH) )
  {
    mp3.playWithFileName(0x01, 1);     // Scream!
    last_scream = millis();
  }

  if (is_eye_covered && !is_eye_covered_prev)
  {
    rf.stopListening();
    rf.write( &eye_covered, sizeof(char) );
    rf.startListening();

    is_eye_covered_prev = true;
    indicateBlink();

    #ifdef DEBUG
    Serial.print(F("Sent: "));
    Serial.println(eye_covered);
    #endif
  }
  else if (!is_eye_covered && is_eye_covered_prev)
  {
    rf.stopListening();
    rf.write( &eye_uncovered, sizeof(char) );
    rf.startListening();

    is_eye_covered_prev = false;
    indicateBlink();

    #ifdef DEBUG
    Serial.print(F("Sent: "));
    Serial.println(eye_uncovered);
    #endif
  }
}


void indicateBlink(void)
{
  digitalWrite(LED_BUILTIN, HIGH);
  delay(10);
  digitalWrite(LED_BUILTIN, LOW);
}
