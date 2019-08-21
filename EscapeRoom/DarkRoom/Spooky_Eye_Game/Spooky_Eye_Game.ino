/* Last modified: 7 Aug 2019.
 *  
 * Moved to Arduino Mega (need more SRAM).
 * Removed DOWN gesture.
 * Added "auto-reset" as workaround for MP3 shield random audio problem.
 *  
 * Credit to Nelsen Edbert, Steven Rachman for Eye graphics.
*/

#include <EEPROM.h>
#include "EyeDisplay.h"
#include "HC_SR04.h"
#include <Adafruit_NeoPixel.h>
#include <SparkFun_APDS9960.h>
#include <RedMP3.h>

#define WAIT_AUDIO(del) delay(del)     // Use this macro to differentiate audio-wait delay from other delays.

// Pin assignemnt for HC-SR04, MP3 module, Neopixel.
#define MP3_TX   15
#define MP3_RX   14    // Not all pins on the Arduino Mega supports change interrupts (hence Software serial).
#define HC_TRIG  2
#define HC_ECHO  3
#define LED_PIN  9
#define RAND_PIN A7
#define RESET    16    // I/O for "auto-reset".

#define NUM_LED 12     // Number of neopixel LEDs.
#define VOLUME  30     // Volume of the MP3 shield. Max is 30.

// Object instantiation.
EyeDisplay eye(-1);
SparkFun_APDS9960 apds = SparkFun_APDS9960();
HC_SR04 hc(HC_TRIG, HC_ECHO);
MP3 mp3(MP3_RX, MP3_TX);
Adafruit_NeoPixel pixels(NUM_LED, LED_PIN, NEO_GRB + NEO_KHZ800);


typedef enum {IDLE, ALERT} State;
State state = IDLE;

constexpr uint8_t nearThreshold = 30;         // Can tweak.
uint8_t proximityData = 0;
unsigned long lastAlert;
const unsigned long alertDuration = 1000;     // Can tweak.

unsigned long lastBlink = 0;
unsigned long lastMove = 0;
unsigned long lastAsk = 0;
const unsigned long minBlinkInterval = 5000;  // Can tweak.
const unsigned long maxMoveInterval = 1300;   // Can tweak.
const unsigned long askInterval = 4500;       // Can tweak. This is so that the 7th audio (007 WHAT) doesn't get cut.
const int blinkFreq = 40;                     // Can tweak.
const int moveFreq = 30;                      // Can tweak.

const float hcNearThreshold = 45;    // Can tweak. Distance in cm.
const byte sequenceLen = 5;          // Can tweak. Length of direction sequence.

typedef enum {UP, LEFT, RIGHT, NOPE} Direction;   // DOWN removed (hard to do DOWN gesture when the mask is hung on the wall).
Direction dirSequence[sequenceLen];    // Array to store random direction sequence.


//------------------- Body -------------------//

void setup()
{
  // Auto-reset mechanism: if this is the first time the board is powered (i.e. power-on-reset), do an
  // external reset on the board to prevent MP3 shield random audio issue.
  int reading = 0;
  EEPROM.get(0, reading);
  if (reading == 0)
  {
    EEPROM.put(0, 1);     // Mark that auto-reset has been made.
    pinMode(RESET, OUTPUT);
    digitalWrite(RESET, LOW);
    while (1);
  }
  EEPROM.put(0, 0);     // Reset the EEPROM flag for the next power-on.

  // Set things up. Nothing related to the game flow here.
  randomSeed(analogRead(RAND_PIN));      // Random seed for Idle state.

  // Initialize MP3 module.
  mp3.begin();
  mp3.pause();      // Need this instruction!!!!
  delay(10);
  mp3.setVolume(VOLUME);

  // Enable gesture sensor and proximity sensor, with no interrupt.
  apds.init();
  apds.enableGestureSensor(false);
  apds.setProximityGain(PGAIN_2X);
  apds.enableProximitySensor(false);

  // Initiate eye.
  eye.begin();
  eye.clear();

  pinMode(13, OUTPUT);    // For indicator & debugging.
  digitalWrite(13, LOW);

  delay(12000);    // Delay for 12 secs for game master to put the mask back into position.
  indicateBlink(50);

  // Initialize neopixel, blink green to show that the game is ready.
  pixels.begin();
  
  for (int i = 0; i < NUM_LED; i++)
    pixels.setPixelColor(i, pixels.Color(0, 150, 0));
  pixels.show();
  delay(400);
  
  pixels.clear();
  pixels.show();


  //********** End of Setup **********

  //********** Beginning of game. Wait till player gets close, meanwhile the eye idles. **********
  setIdle();

  // Garage HC-SR04 sensor acts weird when no object in range. Need to check whether the sensor actually detects something.
  // The sensor needs to report 3 consecutive "near" readings in order for the game to proceed.
  do
  {
    delay(100);

    if (hc.distance() > hcNearThreshold)
      continue;
    if (hc.distance() > hcNearThreshold)
      continue;
  }
  while (hc.distance() > hcNearThreshold);


  //********** Player gets near, intro. **********
  mp3.playWithFileName(0x01, 8);     // Audio: Jumpscare.
  indicateBlink(70);
  delay(80);           // To syncronize when the eyes pop out and when the jumpscare begins.

  setAlert();

  // Show some neopixel effect.
  // Fast blink...
  for (byte i = 0; i < 10; i++)
  {
    showLED(255, 0, 0);
    delay(100);
    showLED(0, 0, 0);
    delay(100);
  }

  // Fade to dim.
  for (byte i = 250; i >= 50; i--)
  {
    showLED(i, 0, 0);
    delay(10);
  }

  // Play intro...
  mp3.playWithFileName(0x01, 1);     // Audio: Intro.

  for (byte i = 0; i < 2; i++)
  {
    delay(1200);
    eye.blink();
    while (eye.isIrisMoving() || eye.isTopMoving())
      eye.update();
  }

  WAIT_AUDIO(7500);


  //********** Main game. First round is a troll, second round gives legit clue. **********
  byte currentRound = 1;

  // currentRound = 1  -->  Troll round.
  // currentRound = 2  -->  Real round.

  while (currentRound <= 2)
  {
    // Generate random direction sequences.
    for (byte i = 0; i < sequenceLen; i++)
      dirSequence[i] = random(0, 3); // Randomize between UP,LEFT,RIGHT (0,1,2).

    // Show the random sequences using movement of the eyes.
    for (byte i = 0; i < sequenceLen; i++)
    {
      indicateBlink(50);
      showGesture(dirSequence[i]);
      delay(200);
    }

    // Check for player's input.
    clearGestureBuffer();
    byte count = 0;

    while (count < sequenceLen)
    {
      if (apds.isGestureAvailable())
      {
        Direction inputGesture = readGesture();

        if (inputGesture == NOPE)   // Ignore if the read input is DOWN.
          continue;

        showGesture(inputGesture);

        // If input gesture is incorrect, quit the round immediately.
        if (inputGesture != dirSequence[count])
          break;

        count++;
      }
    }

    delay(1000);

    // If the input is incorrect...
    if (count < sequenceLen)
    {
      mp3.playWithFileName(0x01, random(2, 5));    // There are 3 audio files for "Wrong" (002, 003, 004).
      indicateBlink(800);
      WAIT_AUDIO(3000);

      continue;    // Repeat the game (same round).
    }

    indicateBlink(300);

    // If all the inputs are correct...
    switch (currentRound)
    {
      case 1:
        mp3.playWithFileName(0x01, 5);     // Audio: You've been trolled!
        WAIT_AUDIO(5000);
        break;

      case 2:
        mp3.playWithFileName(0x01, 6);     // Audio: Here's the clue!
        WAIT_AUDIO(8500);
    }

    currentRound++;
  }

  setIdle();
  clearGestureBuffer();
}


void loop()
{
  //********** Game finished. Need to reset manually. Meanwhile keep tracking and responding to gestures. **********

  apds.readProximity(proximityData);
  apds.clearProximityInt();

  if (proximityData > nearThreshold)
  {
    setAlert();

    if (apds.isGestureAvailable())
    {
      switch (apds.readGesture())
      {
        case DIR_UP:
          eye.moveIris(0, 20); break;
        case DIR_DOWN:
          eye.moveIris(0, -20); break;
        case DIR_LEFT:
          eye.moveIris(25, 0); break;
        case DIR_RIGHT:
          eye.moveIris(-25, 0); break;
        default:
          eye.moveIris(0, 0);
      }

      if (random(4) == 0 && (millis() - lastAsk > askInterval))
      {
        mp3.playWithFileName(0x01, 7);     // Audio: WHAT? What else?
        lastAsk = millis();
      }
    }

  }
  else if (millis() - lastAlert > alertDuration)
    setIdle();

  if (state == IDLE)
    runIdle();

  eye.update();
}


//------------------- Function Definitions -------------------//

void setIdle()
{
  state = State::IDLE;

  eye.setIrisSpeed(1.5);    // Iris speed.
  eye.setTopSpeed(18);      // Blinking speed.
}


void setAlert()
{
  state = State::ALERT;
  lastAlert = millis();

  eye.setIrisSpeed(4);
  eye.moveIris(0, 0);
  eye.moveTop(EyeDisplay::top_d0);
  while (eye.isIrisMoving() || eye.isTopMoving()) {
    eye.update();
  }
}


void runIdle()
{
  if (millis() - lastBlink > minBlinkInterval && random(blinkFreq) == 0) {
    eye.blink();
    lastBlink = millis();
  }
  if (millis() - lastMove > maxMoveInterval || random(moveFreq) == 0) {
    eye.moveIris(random(-7, 8), random(-6, 7));
    lastMove = millis();
  }
}


void showGesture(Direction dir)
{
  switch (dir)
  {
    case UP:
      eye.moveIris(0, -15); break;
    //    case DOWN:
    //      eye.moveIris(0, 15); break;
    case LEFT:
      eye.moveIris(-20, 0); break;
    case RIGHT:
      eye.moveIris(20, 0); break;
  }

  for (byte i = 0; i < 6; i++)
  {
    eye.update();
    showLED(50 + 20 * i, 0, 0);
  }

  delay(200);

  eye.moveIris(0, 0);

  for (byte i = 0; i < 6; i++)
  {
    eye.update();
    showLED(150 - 20 * i, 0, 0);
  }
}


Direction readGesture()
{
  switch (apds.readGesture()) {
    case DIR_UP:
      return NOPE;   // Replaced DOWN with NOPE.
    case DIR_DOWN:
      return UP;
    case DIR_LEFT:
      return RIGHT;
    case DIR_RIGHT:
      return LEFT;
  }
}


void showLED(byte red, byte green, byte blue)
{
  for (int i = 0; i < NUM_LED; i++)
    pixels.setPixelColor(i, pixels.Color(red, green, blue));

  pixels.show();
}


// Clear the gesture buffer (in case gestures are registered before the game starts.
void clearGestureBuffer()
{
  while (apds.isGestureAvailable())
    apds.readGesture();
}


void indicateBlink(unsigned long del)
{
  digitalWrite(13, HIGH);
  delay(del);
  digitalWrite(13, LOW);
}
