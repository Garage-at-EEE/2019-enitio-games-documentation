#define TRIAL_CAMP_NUM_VOTERS 10
#define MAX_SUPPORTED_PLAYERS 20

const unsigned long MAX_ID = 0xF000;
const unsigned long MIN_GM_ID = 0xE000;
const unsigned long MIN_PLAYER_ID = 0xA000;

unsigned long MIN_OG_ID [12] = {0XA029,    0XA488,   0XAA3E,  0XC003,  0XC5E2,  0XCA41, 0XB0B2, 0XB50D, 0XBAC0,    0XD050,   0XD757,  0XDAA3};
// Drachen_1,Drachen_2,Drachen_3,Alatar_1,Alatar_2,Alatar_3, Eva_1, Eva_2, Eva_3, Invicta_1,Invicta_2,Invicta_3

unsigned long MAX_OG_ID [12] = { 0XA470,   0XAA23,   0XAFE9,  0XC576,  0XC9C2,  0XCE52, 0XB498, 0XBA96, 0XBFE4,    0XD702,   0XDA67,  0xDFBC};
// Drachen_1,Drachen_2,Drachen_3,Alatar_1,Alatar_2,Alatar_3, Eva_1, Eva_2, Eva_3, Invicta_1,Invicta_2,Invicta_3


int choice = 0; //For the range of ID

int selPin = A1;
int confirmPin = A2;
int pressed = 0;

/*------------ RF24 STUFF ---------------*/
#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <RF24.h>

#define CE 7
#define CSN 9 //pin 8 being used by en pin of stepper motor

#define CHANNEL 124

byte addresses[][6] = {"0", "1"};
RF24 radio(CE, CSN);

// On Arduino MEGA: Pins 50, 51, 52, 53 will become unavailable
// On Arduino Uno: Pins 10, 11, 12, 13 will become unavailable

struct data
{
  unsigned long id = 0;
  unsigned long fid = 0;
  char text[25] = {'\0'};
  int health = 0;
  int mana = 0;
  int attack = 0;
  int progress = 0;
  int stamp[7] = {0};
};
typedef struct data message;
message Message;

/*------------ MOTOR STUFF ---------------*/

#define MOTOR_DELAY_IN_MS 20

#include <BasicStepperDriver.h>

#define STEP_PER_REV 200
#define GEAR_RATIO 1
#define RADIUS_IN_CM 3.3 //3.475 Latest(WhiteCar); 3.39

#define MOTOR_RPM 80 //Ori is 50
#define MICROSTEPS 1

#define ANGLE_PER_STEP (2.0 * PI/(STEP_PER_REV * GEAR_RATIO))
#define DIST_PER_STEP ((RADIUS_IN_CM)*(ANGLE_PER_STEP))

#define NUM_STEP 1

#define RIGHT_MOTOR_DIR 6
#define RIGHT_MOTOR_STEP 3

#define LEFT_MOTOR_DIR 5
#define LEFT_MOTOR_STEP 2

#define MOTOR_TO_CENTER_IN_CM 10.34 //10.12 Latest(WhiteCar)
#define TURN_ARC (MOTOR_TO_CENTER_IN_CM)*(PI/2.0) //90 degree turn. radius times radian, and the radian is (90/180)*pi

BasicStepperDriver rightStepper(STEP_PER_REV, RIGHT_MOTOR_DIR, RIGHT_MOTOR_STEP);
BasicStepperDriver leftStepper(STEP_PER_REV, LEFT_MOTOR_DIR, LEFT_MOTOR_STEP);

enum Command {
  FRONT, BACK, LEFT, RIGHT, STOP
};

int enPin = 8; //Active LOW, LOW to enable it

/*--------------- BLUETOOTH STUFF ---------------*/

const int BTState = 2;
int state = 0;

unsigned long currentTime;
unsigned long pastTime;

int currentCoordinates[2]; //(x, y) in coordinates
int pastCoordinates[2]; //(x, y) in coordinates

/*-------------- GAME AREA ---------------*/

#define CELL_WIDTH  30.5
#define CELL_LENGTH 30

#define ROW_COUNT 7
#define COL_COUNT 4

#define X 0
#define Y 1

#define INIT_X 0
#define INIT_Y 0

/*-------------- BOMB STUFF ---------------*/

bool bombGrid[COL_COUNT][ROW_COUNT];

const bool pattern1[COL_COUNT][ROW_COUNT] =
{
  {0, 0, 1, 0, 0, 0, 1 },//Car is at the top left
  {0, 0, 1, 1, 0, 0, 0 },
  {1, 0, 0, 0, 0, 1, 0 },
  {0, 1, 0, 0, 1, 0, 1 }
};

const bool pattern2[COL_COUNT][ROW_COUNT] =
{
  {0, 0, 0, 1, 0, 0, 0 },
  {0, 0, 1, 0, 0, 0, 1 },
  {1, 0, 0, 0, 1, 1, 0 },
  {0, 1, 0, 0, 0, 0, 1 }
};

int numBombsAroundMe;

#define BOMB_COUNT 16 //Ori is 7

/*-------------- DISPLAY -----------------*/

#define PIXEL_PIN 4
#define PIXEL_COUNT 6

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN);

const uint32_t PIXEL_BLACK = pixels.Color(0, 0, 0);
const uint32_t PIXEL_RED = pixels.Color(255, 0, 0);
const uint32_t PIXEL_GREEN = pixels.Color(0, 255, 0);
const uint32_t PIXEL_YELLOW = pixels.Color(255, 255, 0);
const uint32_t PIXEL_BLUE = pixels.Color(0, 0, 255);
const uint32_t PIXEL_MAGENTA = pixels.Color(255, 0, 255);
const uint32_t PIXEL_WHITE = pixels.Color(255, 255, 255);

#define REVEAL_DELAY 500 //in milliseconds
#define BLINK_DELAY 200

uint32_t zeroPixState;

/* ------------ GAMEPLAY -----------------*/
bool isReady = false;

void setup() {
  Serial.begin(9600);
  Serial.println("Setting up");

  randomSeed(analogRead(A0));
  rightStepper.begin(MOTOR_RPM, MICROSTEPS);
  leftStepper.begin(MOTOR_RPM, MICROSTEPS);
  pinMode(enPin, OUTPUT);
  digitalWrite(enPin, HIGH); //Disable motors

  pinMode(selPin, INPUT_PULLUP);
  pinMode(confirmPin, INPUT_PULLUP);

  pixels.begin();
  pixels.clear();
  pixels.show();

  while (digitalRead(confirmPin))
  {
    delay(5);

    if ((digitalRead(selPin) == 0) && (pressed == 0))
    {
      pressed = 1;
      if (choice < 11)
      {
        choice++;
      } else
      {
        choice = 0;
      }
    }

    if ((digitalRead(selPin) == 1) && (pressed == 1))
    {
      pressed = 0;
    }

    pixels.clear();
    pixels.show();

    //    Serial.print("Choice: ");
    //    Serial.println(choice);

    if (choice < 3) //Drachen
    {
      for (int i = 0; i < (choice + 1); i++)
      {
        pixels.setPixelColor(i, PIXEL_RED);
      }
      pixels.show();
    } else if (choice < 6) //Alatar
    {
      for (int i = 0; i < (choice - 2); i++)
      {
        pixels.setPixelColor(i, PIXEL_BLUE);
      }
      pixels.show();
    } else if (choice < 9) //Eva
    {
      for (int i = 0; i < (choice - 5); i++)
      {
        pixels.setPixelColor(i, PIXEL_GREEN);
      }
      pixels.show();
    } else if (choice < 12) //Invicta
    {
      for (int i = 0; i < (choice - 8); i++)
      {
        pixels.setPixelColor(i, PIXEL_YELLOW);
      }
      pixels.show();
    }
  }

  //memcpy(bombGrid, pattern1, sizeof(pattern1[0][0])*ROW_COUNT*COL_COUNT);
  bombGridSetUp(BOMB_COUNT);
  printBombMap();
  currentCoordinates[X] = INIT_X;
  currentCoordinates[Y] = INIT_Y;

  radio.begin();
  radio.setChannel(CHANNEL);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_1MBPS);
  radio.openReadingPipe(1, addresses[0]);
  radio.startListening();

  initDisplay();

  Serial.println("Ready");
  delay(1000);

  bombCheckRoutine();
}

void loop() {
  Command vote = getVotes();
  goToDestination(vote);
  printBombMap();
}

/*------------------------------------------------------------------------------*/
void bombGridSetUp(int bombCount) {

  /*
     Conditions
     ----------
     1) At least one safe tile on the last row
     2) At least one safe adjacent tiles (left, right, below) next to it
     3) The path must eventually lead to the initial position

     Methodology
     -----------
     1) Create a safe path first all the way to the bottom row
     2) If the bottom row safe cell created in (1) is not the initial position,
          connect the bottom row safe cell to the initial position
          (This is a little bit of cheating, but the participants do not know of this easter egg)
     3) Randomize the rest of the boxes

     Temp Grid Notation
     ------------------
     -1 : Bomb
     0  : Undecided
     1  : Safe

     Where To Notation
     -----------------
     0  : Left
     1  : Right
     2  : Down

  */

  int tempBombGrid[COL_COUNT][ROW_COUNT];
  for (int r = ROW_COUNT - 1; r >= 0; r--) {
    for (int c = 0; c < COL_COUNT; c++) {
      tempBombGrid[c][r] = 0;
    }
  }

  int currentRow = ROW_COUNT - 1;
  int currentCol = random(0, COL_COUNT); //random() excludes the max

  int whereTo;
  int numSafe = 0;

  tempBombGrid[currentCol][currentRow] = 1;
  numSafe++;

  /* Creating the safe path from top to bottom */

  do {
    if ((tempBombGrid[currentCol - 1][currentRow] == 1) || (tempBombGrid[currentCol + 1][currentRow] == 1)) {
      // if both left and right of current box is already safe
      whereTo = 2;
    } else if ((currentCol == 0) || (tempBombGrid[currentCol - 1][currentRow] == 1)) {
      // if the left box is already safe, or the current box is on the extreme left
      whereTo = random(1, 3); //whereTo = {1, 2}
    } else if ((currentCol == COL_COUNT - 1) || (tempBombGrid[currentCol + 1][currentRow] == 1)) {
      // if the right box is already safe, or the current box is on the extreme right
      whereTo = abs(2 * random(0, 3) - 2); //whereTo = {2, 0, 2}
    } else {
      whereTo = random(0, 3); //whereTo = {0, 1, 2}
    }

    switch (whereTo) {
      case 0:
        currentCol--;
        tempBombGrid[currentCol][currentRow] = 1;
        numSafe++;
        break;
      case 1:
        currentCol++;
        tempBombGrid[currentCol][currentRow] = 1;
        numSafe++;
        break;
      case 2:
        currentRow--;
        tempBombGrid[currentCol][currentRow] = 1;
        numSafe++;
        break;
    }
  } while (currentRow > 0);

  /* Creating the safe path from initial positon to the previous path */

  if (currentCol != INIT_X) {
    if ((abs(currentCol - INIT_X) == 1) || (tempBombGrid[INIT_X][1] == 1)) {
      tempBombGrid[INIT_X][0] = 1;
      numSafe++;
    } else if (currentCol < INIT_X) {
      do {
        currentCol++;
        tempBombGrid[currentCol][0] = 1;
        numSafe++;
      } while (currentCol < INIT_X);
    } else {
      do {
        currentCol--;
        tempBombGrid[currentCol][0] = 1;
        numSafe++;
      } while (currentCol > INIT_X);
    }
  }

  /* Randomizing bomb locations */

  int bombLeft = bombCount;
  int numBoxes = ROW_COUNT * COL_COUNT;

  numBoxes -= numSafe;

  double probThreshold = bombLeft / numBoxes;

  int maxIter = 5000;
  int iter = 0;

  do {
    for (int c = 0; c < COL_COUNT; c++) {
      if (bombLeft == 0) {
        break;
      }
      for (int r = 0; r < ROW_COUNT; r++) {
        if (bombLeft == 0) {
          break;
        }

        if (tempBombGrid[c][r] == 0) {
          if (probThreshold < random(0, 100) / 100.0) {
            tempBombGrid[c][r] = -1;
            bombLeft--;
            Serial.print("bombLeft = ");
            Serial.print(bombLeft);
            Serial.print(", at (");
            Serial.print(r);
            Serial.print(", ");
            Serial.print(c);
            Serial.println(")");
          }
        }

        numBoxes--;
        probThreshold = (double)((double)bombLeft / (double)numBoxes);
      }
    }

    iter++;
  } while ((bombLeft > 0) && (iter < maxIter));

  /*setting bool array*/

  for (int c = 0; c < COL_COUNT; c++) {
    for (int r = 0; r < ROW_COUNT; r++) {
      if (tempBombGrid[c][r] < 0) {
        bombGrid[c][r] = true;
      } else
        bombGrid[c][r] = false;
    }
  }
}

void printBombMap() {
  Serial.print(" C|");
  for (int c = 0; c < COL_COUNT; c++) {
    Serial.print(c);
    Serial.print("|");
  }
  Serial.println("");
  for (int r = ROW_COUNT - 1; r >= 0 ; r--) {
    Serial.print("R");
    Serial.print(r);
    Serial.print("|");
    for (int c = 0; c < COL_COUNT; c++) {
      if (bombGrid[c][r]) {
        Serial.print("X|");
      } else if ((currentCoordinates[X] == c) && (currentCoordinates[Y] == r)) {
        Serial.print("@|");
      }
      else {
        Serial.print("O|");
      }
    }
    Serial.println("");
  }
}
/*------------------------------------------------------------------------------*/

void bombedDisplay() {
  for (int k = 0; k < 10; k++) {
    for (int i = 0; i < PIXEL_COUNT; i++) {
      pixels.setPixelColor(i, PIXEL_RED);
    }
    pixels.show();
    delay(BLINK_DELAY);
    pixels.clear();
    pixels.show();
    delay(BLINK_DELAY);
  }
}

void survivedDisplay() {
  for (int k = 0; k < 10; k++) {
    for (int i = 0; i < PIXEL_COUNT; i++) {
      pixels.setPixelColor(i, PIXEL_GREEN);
    }
    pixels.show();
    delay(BLINK_DELAY);
    pixels.clear();
    pixels.show();
    delay(BLINK_DELAY);
  }
}

void initDisplay() {

  for (int i = 0; i < PIXEL_COUNT; i++) {
    pixels.setPixelColor(i, PIXEL_WHITE);
    pixels.show();
    delay(BLINK_DELAY);
  }

  // light up white one by one

  for (int i = 0; i < PIXEL_COUNT; i++) {
    pixels.setPixelColor(i, PIXEL_GREEN);
  }
  pixels.show();

  // then all light up green

}

void gmDisplay() {
  //pixels.clear();
  //pixels.show();

  pixels.setPixelColor(0, PIXEL_WHITE);
  pixels.show();
}

void newVoterDisplay() {
  //pixels.clear();
  //pixels.show();

  pixels.setPixelColor(0, PIXEL_BLUE);
  pixels.show();
}

void repeatedVoterDisplay() {
  //pixels.clear();
  //pixels.show();

  pixels.setPixelColor(1, PIXEL_MAGENTA);
  pixels.show();
}

void radioAvailDisplay() {
  //pixels.clear();
  //pixels.show();

  pixels.setPixelColor(0, PIXEL_YELLOW);
  pixels.show();
}

void dangerDisplay(bool bombAroundMe[]) {

  /* NeoPixel Numbering

     FRONT
     [2] [1] [0]

     [3] [4] [5]
     BACK
  */

  int frontBomb = 0;
  int backBomb = 0;

  for (int i = 0; i < 3; i++) {
    frontBomb += bombAroundMe[i];
    Serial.print("Front Bomb: ");
    Serial.println(frontBomb);
  }

  for (int i = 3; i < 6; i++) {
    backBomb += bombAroundMe[i];
    Serial.print("Back Bomb: ");
    Serial.println(backBomb);
  }

  bool redPix[PIXEL_COUNT];

  for (int i = 0; i < 6; i++) {
    redPix[i] = false;
  }

  switch (frontBomb) {
    case 1:
      redPix[random(0, 3)] = true;
      break;
    case 2:
      int rand1 = random(0, 3);
      int rand2 = random(0, 3);
      while (rand1 == rand2) {
        rand2 = random(0, 3);
      }
      redPix[rand1] = true;
      redPix[rand2] = true;
      break;
    case 3:
      break;
  }

  switch (backBomb) {
    case 1:
      redPix[random(3, 6)] = true;
      break;
    case 2:
      int rand1 = random(3, 6);
      int rand2 = random(3, 6);
      while (rand1 == rand2) {
        rand2 = random(3, 6);
      }
      redPix[rand1] = true;
      redPix[rand2] = true;
      break;
    case 3:
      break;
  }

  for (int i = 0; i < 6; i++) {
    if (redPix[i]) {
      pixels.setPixelColor(i, PIXEL_RED);
      pixels.show();
    } else {
      pixels.setPixelColor(i, PIXEL_GREEN);
      pixels.show();
    }
    delay(REVEAL_DELAY);
  }

  if (redPix[0]) {
    zeroPixState = PIXEL_RED;
  } else {
    zeroPixState = PIXEL_GREEN;
  }
}

/*------------------------------------------------------------------------------*/

void goToDestination(Command command) {

  Serial.println("Going");

  switch (command) {
    case FRONT:
      Serial.println("f");
      goFront(CELL_LENGTH);
      updateCoordinates(FRONT);
      break;
    case BACK:
      Serial.println("b");
      goBack(CELL_LENGTH);
      updateCoordinates(BACK);
      break;
    case LEFT:
      Serial.println("l");
      goLeft(CELL_WIDTH);
      updateCoordinates(LEFT);
      break;
    case RIGHT:
      Serial.println("r");
      goRight(CELL_WIDTH);
      updateCoordinates(RIGHT);
      break;
    case STOP:
      /*rightStepper.setSpeed(0);
        leftStepper.setSpeed(0);
        delay(1);
        rightStepper.setSpeed(MOTOR_SPEED);
        leftStepper.setSpeed(MOTOR_SPEED);*/
      break;
  }

  bombCheckRoutine();
  //delay(1000);
}

void goFront(double distance) {
  Serial.println("stepping front");
  Serial.println(distance);

  long totalStep = getNumStep(distance);// / 2.0;

  Serial.println(totalStep);

  digitalWrite(enPin, LOW);

  for (int i = 0; i < totalStep; i += NUM_STEP) {
    //Serial.print("stepping ");
    //Serial.println(i);
    rightStepper.move(NUM_STEP);
    leftStepper.move(-NUM_STEP);
    delay(MOTOR_DELAY_IN_MS);
  }

  digitalWrite(enPin, HIGH);
}

void goBack(double distance) {
  long totalStep = getNumStep(distance);// / 2.0;
  digitalWrite(enPin, LOW);
  for (int i = 0; i < totalStep; i += NUM_STEP) {
    //Serial.print("stepping ");
    //Serial.println(i);
    rightStepper.move(-NUM_STEP);
    leftStepper.move(NUM_STEP);
    delay(MOTOR_DELAY_IN_MS);
  }
  digitalWrite(enPin, HIGH);
}

void goRight(double distance) {
  turnRight();
  long totalStep = getNumStep(distance);// / 2.0;
  digitalWrite(enPin, LOW);
  for (int i = 0; i < totalStep; i += NUM_STEP) {
    rightStepper.move(NUM_STEP);
    leftStepper.move(-NUM_STEP);
    delay(MOTOR_DELAY_IN_MS);
  }
  digitalWrite(enPin, HIGH);
  turnLeft();
}

void goLeft(double distance) {
  turnLeft();
  long totalStep = getNumStep(distance);// / 2.0;
  digitalWrite(enPin, LOW);
  for (int i = 0; i < totalStep; i += NUM_STEP) {
    rightStepper.move(NUM_STEP);
    leftStepper.move(-NUM_STEP);
    delay(MOTOR_DELAY_IN_MS);
  }
  digitalWrite(enPin, HIGH);
  turnRight();
}

void turnRight() {
  long totalStep = getNumStep(TURN_ARC);
  digitalWrite(enPin, LOW);
  for (int i = 0; i < totalStep; i += NUM_STEP) {
    rightStepper.move(-NUM_STEP);
    leftStepper.move(-NUM_STEP);
    delay(MOTOR_DELAY_IN_MS);
  }
  digitalWrite(enPin, HIGH);
}

void turnLeft() {
  long totalStep = getNumStep(TURN_ARC);
  digitalWrite(enPin, LOW);
  for (int i = 0; i < totalStep; i += NUM_STEP) {
    rightStepper.move(NUM_STEP);
    leftStepper.move(NUM_STEP);
    delay(MOTOR_DELAY_IN_MS);
  }
  digitalWrite(enPin, HIGH);
}

long getNumStep(double distance_in_cm) {
  //DIST_PER_STEP is in cm

  long out = (long)((double)distance_in_cm / (double)DIST_PER_STEP);

  Serial.println((double)distance_in_cm / (double)DIST_PER_STEP);

  return out;
}

/*------------------------------------------------------------------------------*/

void updateCoordinates(Command command) {
  switch (command) {
    case FRONT:
      currentCoordinates[Y]++;
      break;
    case BACK:
      currentCoordinates[Y]--;
      break;
    case LEFT:
      currentCoordinates[X]--;
      break;
    case RIGHT:
      currentCoordinates[X]++;
      break;
    case STOP:
      break;
  }
}

void bombCheckRoutine() {
  if (isThereBombHere(currentCoordinates[X], currentCoordinates[Y])) {
    bombedDisplay();
    if (currentCoordinates[Y] > 0) {
      goToDestination(BACK);
    } else if (currentCoordinates[X] > 0) {
      goToDestination(RIGHT);  //in case the car goes backwards all the way to the bottom row
    } else {
      goToDestination(LEFT);   //in case the car goes backwards all the way to the bottom row AND the rightmost column
    }
  } else {
    survivedDisplay();
    checkForBombsAroundMe(currentCoordinates[X], currentCoordinates[Y]);
  }
}

bool isThereBombHere(int Xcoor, int Ycoor) {
  if ((Xcoor > -1) && (Xcoor < COL_COUNT) && (Ycoor > -1) && (Ycoor < COL_COUNT)) {
    return bombGrid[Xcoor][Ycoor];
  } else {
    return false;
  }
}

void checkForBombsAroundMe(int Xcoor, int Ycoor) {

  bool bombCount[6];

  for (int i = 0; i < 6; i++) {
    bombCount[i] = 0;
  }

  int j = 1;
  for (int i = -1; i <= 1; i++) {
    bombCount[i + 1] = isThereBombHere(Xcoor + i, Ycoor + j);
  }

  j = -1;
  for (int i = -1; i <= 1; i++) {
    bombCount[i + 4] = isThereBombHere(Xcoor + i, Ycoor + j);
  }

  dangerDisplay(bombCount);
}

Command getVotes() {

  Serial.println("Getting votes");

  int irVotes[4] = {0};
  bool isVotingComplete = false;
  Command vote = STOP;
  int playerCount = 0;
  bool gmOverride = false;

  unsigned long playerID;
  unsigned long voters[MAX_SUPPORTED_PLAYERS] = {0};
  char command;

  bool firstVote = true;

  unsigned long lastBlink;

  do {
    command = '0';
    playerID = 0;
    gmOverride = false;

    if (radio.available()) {
      while (radio.available()) {
        radio.read(&Message, sizeof(Message));
      }
      playerID = Message.id;
      Serial.println(playerID);

      radioAvailDisplay();

    } else {
      playerID = 0;
    }



    if ((playerID >= MIN_PLAYER_ID) && (playerID <= MAX_ID))
    {
      if (playerID >= MIN_GM_ID) {

        gmDisplay();
        lastBlink = millis();

        command = Message.text[0];
        gmOverride = true;
        Serial.println("GM Overriding");
      } else if ((playerID >= MIN_OG_ID[choice]) && (playerID <= MAX_OG_ID[choice]))
      {

        if (uniqueVoter(playerID, voters))
        {
          newVoterDisplay();
          lastBlink = millis();

          Serial.print("This is the ");
          Serial.print(playerCount + 1);
          Serial.println(" th voter");


          command = Message.text[0];

          Serial.print("Player ");
          Serial.print(playerID);
          Serial.print(" voted ");
        }
      } //else {
      //        //repeated voter!
      //
      //        repeatedVoterDisplay();
      //        lastBlink = millis();
      //
      //        Serial.println("Repeated voter!");
      //        command = '0';
      //      }

      switch (command) {
        case 'u':
          Serial.println("front");
          irVotes[0] += 1;
          voters[playerCount] = playerID;
          playerCount += 1;
          break;
        case 'd':
          Serial.println("back");
          if (currentCoordinates[Y] > 0) {
            irVotes[1] += 1;
            voters[playerCount] = playerID;
            playerCount += 1;
          }
          break;
        case 'l':
          Serial.println("left");
          if (currentCoordinates[X] > 0) {
            irVotes[2] += 1;
            voters[playerCount] = playerID;
            playerCount += 1;
          }
          break;
        case 'r':
          Serial.println("right");
          if (currentCoordinates[X] < COL_COUNT - 1) {
            irVotes[3] += 1;
            voters[playerCount] = playerID;
            playerCount += 1;
          }
          break;
        case 's':
          if (gmOverride) {
            Serial.println("GM: complete");
            isVotingComplete = true;
          }
          break;
        default:
          Serial.println("NOTHING");
      }

      Serial.println("Voters: ");
      for (int i = 0; i < MAX_SUPPORTED_PLAYERS; i++) {
        Serial.println(voters[i]);
      }

      Serial.println("Votes: ");

      for (int i = 0; i < 4; i++) {
        Serial.println(irVotes[i]);
      }
    }


    //for testing only
    if (playerCount == TRIAL_CAMP_NUM_VOTERS) {
      isVotingComplete = true;
    }

    if (millis() - lastBlink > REVEAL_DELAY) {
      pixels.setPixelColor(0, zeroPixState);
      pixels.show();
    }

  } while (!isVotingComplete);

  Serial.println("Voting completed");

  /* Find the direction with the maximum votes
     If a tie occurs, randomize.
  */

  int maxIdx = 0;
  int tieIdx = -1;
  int tie2ndIdx = -1;
  int tie3rdIdx = -1;

  for (int i = 0; i < 4; i++) {
    if (irVotes[i] > irVotes[maxIdx]) {
      maxIdx = i;
      tieIdx = -1;
      tie2ndIdx = -1;
      tie3rdIdx = -1;
    } else if ((irVotes[i] == irVotes[maxIdx]) && (tieIdx < 0)) {
      // if there is a tie between the max and the current value (1st tie, i.e. 2 same votes)
      tieIdx = i;
      tie2ndIdx = -1;
      tie3rdIdx = -1;
    } else if ((irVotes[i] == irVotes[maxIdx]) && (!(tieIdx < 0)) && (tie2ndIdx < 0)) {
      // if there is a tie between the max and the current value (2nd tie, i.e. 3 same votes)
      tie2ndIdx = i;
      tie3rdIdx = -1;
    } else if ((irVotes[i] == irVotes[maxIdx]) && (!(tieIdx < 0)) && (!(tie2ndIdx < 0)) && (tie3rdIdx < 0)) {
      // if there is a tie between the max and the current value (3rd tie, i.e. 4 same votes)
      tie3rdIdx = i;
    }
  }

  int fate = 0;

  if (!(tie3rdIdx < 0)) {
    fate = random(0, 4); //randome integer in [0, 4) i.e. {0, 1, 2, 3}
  } else if (!(tie2ndIdx < 0)) {
    fate = random(0, 3); //randome integer in [0, 4) i.e. {0, 1, 2, 3}
  } else if (!(tieIdx < 0)) {
    fate = random(0, 2); //randome integer in [0, 4) i.e. {0, 1, 2, 3}
  }

  switch (fate) {
    case 0:
      maxIdx = maxIdx;
      break;
    case 1:
      maxIdx = tieIdx;
      break;
    case 2:
      maxIdx = tie2ndIdx;
      break;
    case 3:
      maxIdx = tie3rdIdx;
      break;
  }

  switch (maxIdx) {
    case 0:
      vote = FRONT;
      break;
    case 1:
      vote = BACK;
      break;
    case 2:
      vote = LEFT;
      break;
    case 3:
      vote = RIGHT;
      break;
    default:
      vote = STOP;
  }

  Serial.print("Voting result: ");
  Serial.println(vote);

  return vote;
}

void resetVotes(int irVotes[]) {
  for (int i = 0; i < 4; i++) {
    irVotes[i] = 0;
  }
}

bool uniqueVoter(unsigned long playerID, unsigned long votedPlayers[]) {
  for (int i = 0; i < MAX_SUPPORTED_PLAYERS; i++) {
    if (playerID == votedPlayers[i]) {
      return false;
    }
  }

  return true;
}
