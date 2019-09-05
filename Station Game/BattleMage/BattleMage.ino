/////////////////////Neopixels Declaration Start/////////////////////
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#define PIN            6
#define NUMPIXELS    216

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

/*
  Block numbering     Neopixel numbering
    |16|17|18|           |16|17|18|
    |13|14|15|           |15|12|11|
    |10|11|12|           |14|13|10|

    |01|02|03|           |01|04|05|
    |04|05|06|           |02|03|06|
    |07|08|09|           |09|08|07|
COL: 1  2  3
  
  To reduce the length of wire used, the neopixels were physically fixed in this fashion.
  BlockInit specifies the first number of the neopixels in each block.
*/

int blockInit[18] = {0, 36, 48, 12, 24, 60, 96, 84, 72, 156, 144, 108, 168, 132, 120, 180, 192, 204};

const uint32_t PIXEL_BLUE   = pixels.Color(0, 0, 150);
const uint32_t PIXEL_PURPLE = pixels.Color(150, 0, 150);
const uint32_t PIXEL_GREEN  = pixels.Color(0, 150, 0);
const uint32_t PIXEL_RED    = pixels.Color(150, 0, 0);
const uint32_t PIXEL_YELLOW = pixels.Color(150, 150, 0);
const uint32_t PIXEL_CYAN = pixels.Color(0, 150, 150);

const uint32_t PIXEL_CLEAR   = pixels.Color(0, 0, 0);

uint32_t colors[5] = {PIXEL_BLUE, PIXEL_PURPLE, PIXEL_GREEN, PIXEL_YELLOW, PIXEL_CYAN};

int randColor = -1;

/////////////////////Neopixels Declaration Ended/////////////////////

/////////////////////Pins Declaration Start/////////////////////
int rowPin[6] = {9, 10, 11, 2, 3, 4}; //row 1 is from the left
int gndCol[3] = {A0, A1, A2}; //col 1 is from left
int startPin = 12;
/////////////////////Pins Declaration Ended/////////////////////

/////////////////////Patterns Declaration Start/////////////////////
int pattern_loadnear [][3] = {{2, 4, 6}, {3, 4, 5}, {0, 4, 8}, {1, 4, 7}}; //Must specify column size
int pattern_tree[][5] = {{7}, {4}, {0, 2}, {1, 3, 5, 6, 8}};
int pattern_loadfar[][4] = {{13, 15, 16, 17}, {9, 10, 11, 12}, {13, 14, 15, 16}, {10, 11, 13, 14}};
int pattern_star[][4] = {{16}, {13}, {10, 12, 14}, {9, 11, 15, 17}};

int chgPatt = 0;
int swtPatt = 0;
/////////////////////Patterns Declaration Ended/////////////////////

int preVal[18];
int nowVal[18];
int blockBias[6] = {1, 4, 7, 10, 13, 16}; //six rows

int colCount = 0;
int proceed = 1;
int nearNum;
int farNum;

unsigned long preT = 0;
unsigned long nowT = 0;

unsigned long interval = 50; // in ms

int nextGame = 0;

/////////////////////PointSys Declaration Start/////////////////////
int nearScore = 0;
int farScore = 0;
/////////////////////PointSys Declaration Ended/////////////////////

#include <SoftwareSerial.h>
#include "RedMP3.h"

#define MP3_RX 7//RX of Serial MP3 module connect to D7 of Arduino
#define MP3_TX 8//TX to D8, note that D8 can not be used as RX on Mega2560, you should modify this if you donot use Arduino UNO
MP3 mp3(MP3_RX, MP3_TX);

int8_t volume = 0x1e;//0~0x1e (30 adjustable level)
int8_t folderName = 0x01;//folder name must be 01 02 03 04 ...
int8_t fileName = 0x01; // prefix of file name must be 001xxx 002xxx 003xxx 004xxx ...

void setup()
{
  delay(500);

  pinMode(startPin, INPUT_PULLUP);

  for (int i = 0; i < 6; i++)
  {
    pinMode(rowPin[i], INPUT_PULLUP);
  }

  for (int k = 0; k < 3; k++)
  {
    pinMode(gndCol[k], OUTPUT);
    digitalWrite(gndCol[k], HIGH); //Initialisation : To read as not pressed
  }

  for (int i = 0; i < 18; i++)
  {
    preVal[i] = 0; //0 means not pressed
    nowVal[i] = 0;
  }

  pixels.begin();

  pixels.clear();
  pixels.show();

  while (digitalRead(startPin) == 1) //GM haven't press the button yet
  {
    //In the standby mode, the opposite two sides will continously play patterns (Monster Hunter World)
    while (swtPatt < 12)
    {
      for (int z = 0; z < 3; z++)
      {
        for (int i = blockInit[pattern_loadnear[chgPatt][z]]; i < (blockInit[pattern_loadnear[chgPatt][z]] + 12); i++)
        {
          pixels.setPixelColor(i, colors[chgPatt]);
          if (digitalRead(startPin) == 0) //GM Pressed
          {
            break;
          }
        }
      }

      for (int g = 0; g < 4; g++)
      {
        for (int i = blockInit[pattern_loadfar[chgPatt][g]]; i < (blockInit[pattern_loadfar[chgPatt][g]] + 12); i++)
        {
          pixels.setPixelColor(i, colors[4 - chgPatt]);
          if (digitalRead(startPin) == 0) //GM Pressed
          {
            break;
          }
        }
      }

      pixels.show();

      if (digitalRead(startPin) == 0) //GM Pressed
      {
        break;
      }

      delay(1000);

      for (int z = 0; z < 3; z++)
      {
        for (int i = blockInit[pattern_loadnear[chgPatt][z]]; i < (blockInit[pattern_loadnear[chgPatt][z]] + 12); i++)
        {
          pixels.setPixelColor(i, PIXEL_CLEAR);
        }
      }

      for (int g = 0; g < 4; g++)
      {
        for (int i = blockInit[pattern_loadfar[chgPatt][g]]; i < (blockInit[pattern_loadfar[chgPatt][g]] + 12); i++)
        {
          pixels.setPixelColor(i, PIXEL_CLEAR);
        }
      }
      pixels.show();

      if (chgPatt < 3)
      {
        chgPatt++;
      } else
      {
        chgPatt = 0;
      }
      swtPatt++;

      if (digitalRead(startPin) == 0) //GM Pressed
      {
        break;
      }
    }

    chgPatt = 0;

    while ((swtPatt >= 12) && (swtPatt < 14))
    {
      for (int i = blockInit[pattern_tree[chgPatt][0]]; i < (blockInit[pattern_tree[chgPatt][0]] + 12); i++)
      {
        pixels.setPixelColor(i, PIXEL_CYAN);

        if (digitalRead(startPin) == 0) //GM Pressed
        {
          break;
        }
      }

      for (int i = blockInit[pattern_star[chgPatt][0]]; i < (blockInit[pattern_star[chgPatt][0]] + 12); i++)
      {
        pixels.setPixelColor(i, PIXEL_RED);

        if (digitalRead(startPin) == 0) //GM Pressed
        {
          break;
        }
      }

      pixels.show();

      if (digitalRead(startPin) == 0) //GM Pressed
      {
        break;
      }

      delay(1000);
      swtPatt++;
      chgPatt++;

      if (digitalRead(startPin) == 0) //GM Pressed
      {
        break;
      }
    }

    chgPatt = 0;

    if (swtPatt == 14)
    {
      for (int z = 0; z < 2; z++)
      {
        for (int i = blockInit[pattern_tree[2][z]]; i < (blockInit[pattern_tree[2][z]] + 12); i++)
        {
          pixels.setPixelColor(i, PIXEL_CYAN);
          if (digitalRead(startPin) == 0) //GM Pressed
          {
            break;
          }
        }
      }

      for (int p = 0; p < 3; p++)
      {
        for (int i = blockInit[pattern_star[2][p]]; i < (blockInit[pattern_star[2][p]] + 12); i++)
        {
          pixels.setPixelColor(i, PIXEL_RED);
          if (digitalRead(startPin) == 0) //GM Pressed
          {
            break;
          }
        }
      }

      pixels.show();
      delay(1000);
      swtPatt++;
      if (digitalRead(startPin) == 0) //GM Pressed
      {
        break;
      }
    }

    int x = 50;
    int cnt = 0;
    int whichFar = 0;

    while (swtPatt < 26)
    {
      for (int z = 0; z < 5; z++)
      {
        for (int i = blockInit[pattern_tree[3][z]]; i < (blockInit[pattern_tree[3][z]] + 12); i++)
        {
          pixels.setPixelColor(i, pixels.Color(x, 0, 0));
          if (digitalRead(startPin) == 0) //GM Pressed
          {
            break;
          }
        }
      }

      if (cnt == 0)
      {
        for (int z = 0; z < 5; z++)
        {
          for (int i = blockInit[pattern_star[3][whichFar]]; i < (blockInit[pattern_star[3][whichFar]] + 12); i++)
          {
            pixels.setPixelColor(i, pixels.Color(0, 50 + (50 * z), 50 + (50 * z)));
            if (digitalRead(startPin) == 0) //GM Pressed
            {
              break;
            }
          }
          pixels.show();
          delay(20);
        }
        cnt = 1;

      } else if (cnt == 1) //fading out pattern
      {
        for (int z = 0; z < 5; z++)
        {
          for (int i = blockInit[pattern_star[3][whichFar]]; i < (blockInit[pattern_star[3][whichFar]] + 12); i++)
          {
            pixels.setPixelColor(i, pixels.Color(0, 250 - (50 * z), 250 - (50 * z)));
            if (digitalRead(startPin) == 0) //GM Pressed
            {
              break;
            }
          }
          pixels.show();
          delay(20);
        }

        for (int i = blockInit[pattern_star[3][whichFar]]; i < (blockInit[pattern_star[3][whichFar]] + 12); i++)
        {
          pixels.setPixelColor(i, PIXEL_CLEAR);
        }
        cnt = 0;

        if (whichFar < 3)
        {
          whichFar++;
        } else
        {
          cnt = -1;
        }

      }

      swtPatt++;
      x += 20;
      if (digitalRead(startPin) == 0) //GM Pressed
      {
        break;
      }
    }

    if (swtPatt == 26)
    {
      delay(2000);

      swtPatt = 0;

      pixels.clear();

      pixels.show();

      if (digitalRead(startPin) == 0) //GM Pressed
      {
        break;
      }

      delay(2000);
    }
  }

  pixels.clear();
  pixels.show();

  //mp3.setVolume(volume);
  //delay(50);
  // mp3.playWithFileName(folderName,fileName);
  //mp3.playWithIndex(0x01);
  mp3.playWithVolume(0x01, 0x1e);
  delay(50);

  for (int f = 6; f < 9; f++)
  {
    for (int i = blockInit[f]; i < (blockInit[f] + 12); i++)
    {
      pixels.setPixelColor(i, PIXEL_RED);
    }
  }

  for (int f = 15; f < 18; f++)
  {
    for (int i = blockInit[f]; i < (blockInit[f] + 12); i++)
    {
      pixels.setPixelColor(i, PIXEL_RED);
    }
  }

  pixels.show();

  delay(1000);

  for (int f = 6; f < 9; f++)
  {
    for (int i = blockInit[f]; i < (blockInit[f] + 12); i++)
    {
      pixels.setPixelColor(i, PIXEL_CLEAR);
    }
  }

  for (int f = 15; f < 18; f++)
  {
    for (int i = blockInit[f]; i < (blockInit[f] + 12); i++)
    {
      pixels.setPixelColor(i, PIXEL_CLEAR);
    }
  }

  for (int f = 3; f < 6; f++)
  {
    for (int i = blockInit[f]; i < (blockInit[f] + 12); i++)
    {
      pixels.setPixelColor(i, PIXEL_YELLOW);
    }
  }

  for (int f = 12; f < 15; f++)
  {
    for (int i = blockInit[f]; i < (blockInit[f] + 12); i++)
    {
      pixels.setPixelColor(i, PIXEL_YELLOW);
    }
  }

  pixels.show();

  delay(1000);

  for (int f = 3; f < 6; f++)
  {
    for (int i = blockInit[f]; i < (blockInit[f] + 12); i++)
    {
      pixels.setPixelColor(i, PIXEL_CLEAR);
    }
  }

  for (int f = 12; f < 15; f++)
  {
    for (int i = blockInit[f]; i < (blockInit[f] + 12); i++)
    {
      pixels.setPixelColor(i, PIXEL_CLEAR);
    }
  }

  for (int f = 0; f < 3; f++)
  {
    for (int i = blockInit[f]; i < (blockInit[f] + 12); i++)
    {
      pixels.setPixelColor(i, PIXEL_GREEN);
    }
  }

  for (int f = 9; f < 12; f++)
  {
    for (int i = blockInit[f]; i < (blockInit[f] + 12); i++)
    {
      pixels.setPixelColor(i, PIXEL_GREEN);
    }
  }

  pixels.show();

  delay(1000);

  for (int f = 0; f < 3; f++)
  {
    for (int i = blockInit[f]; i < (blockInit[f] + 12); i++)
    {
      pixels.setPixelColor(i, PIXEL_CLEAR);
    }
  }

  for (int f = 9; f < 12; f++)
  {
    for (int i = blockInit[f]; i < (blockInit[f] + 12); i++)
    {
      pixels.setPixelColor(i, PIXEL_CLEAR);
    }
  }

  pixels.show();
}



void loop()
{
  if (proceed == 1)
  {
    digitalWrite(gndCol[colCount], LOW);

    for (int j = 0; j < 6; j++)
    {
      nowVal[blockBias[j] + colCount - 1] = !digitalRead(rowPin[j]); //Since this is a pull up input, so High means not pressed

      if ((digitalRead(rowPin[j]) == 0) && (preVal[(blockBias[j] + colCount - 1)] == 0)) //Haven't pressed before
      {
        randColor = random(0, 5);

        for (int i = blockInit[blockBias[j] + colCount - 1]; i < (blockInit[blockBias[j] + colCount - 1] + 12); i++)
        {
          pixels.setPixelColor(i, colors[randColor]);
        }

        pixels.show();

      } else if ((digitalRead(rowPin[j]) == 1) && (preVal[(blockBias[j] + colCount - 1)] == 1)) //Pressed before
      {
        for (int i = blockInit[blockBias[j] + colCount - 1]; i < (blockInit[blockBias[j] + colCount - 1] + 12); i++)
        {
          pixels.setPixelColor(i, PIXEL_CLEAR);
        }

        pixels.show();
      }
    }

    logic();

    if (nextGame)
    {
      nextGame = 0;
      //Show points with neopixels

      //Reset everything
      for (int k = 0; k < 3; k++)
      {
        digitalWrite(gndCol[k], HIGH); //Initialisation : To read as not pressed
      }

      for (int i = 0; i < 18; i++)
      {
        preVal[i] = 0; //0 means not pressed
        nowVal[i] = 0;
      }

      colCount = 0;

      pixels.clear();
      pixels.show();

      if (nearScore > farScore)
      {
        for (int f = 0; f < nearScore; f++)
        {
          for (int i = blockInit[f]; i < (blockInit[f] + 12); i++)
          {
            pixels.setPixelColor(i, PIXEL_BLUE);
          }

          if (f < farScore)
          {
            for (int j = blockInit[f + 9]; j < (blockInit[f + 9] + 12); j++)
            {
              pixels.setPixelColor(j, PIXEL_YELLOW);
            }
          }

          pixels.show();
          delay(500);
        }
      } else if (nearScore < farScore)
      {
        for (int f = 0; f < farScore; f++)
        {
          if (f < nearScore)
          {
            for (int i = blockInit[f]; i < (blockInit[f] + 12); i++)
            {
              pixels.setPixelColor(i, PIXEL_BLUE);
            }
          }

          for (int j = blockInit[f + 9]; j < (blockInit[f + 9] + 12); j++)
          {
            pixels.setPixelColor(j, PIXEL_YELLOW);
          }

          pixels.show();
          delay(500);
        }
      } else if (nearScore == farScore)
      {
        for (int f = 0; f < nearScore; f++)
        {
          for (int i = blockInit[f]; i < (blockInit[f] + 12); i++)
          {
            pixels.setPixelColor(i, PIXEL_BLUE);
          }

          for (int j = blockInit[f + 9]; j < (blockInit[f + 9] + 12); j++)
          {
            pixels.setPixelColor(j, PIXEL_YELLOW);
          }

          pixels.show();
          delay(500);
        }
      }

      delay(2000);

      pixels.clear();
      pixels.show();

      mp3.playWithVolume(0x01, 0x1e);
      delay(50);

      for (int f = 6; f < 9; f++)
      {
        for (int i = blockInit[f]; i < (blockInit[f] + 12); i++)
        {
          pixels.setPixelColor(i, PIXEL_RED);
        }
      }

      for (int f = 15; f < 18; f++)
      {
        for (int i = blockInit[f]; i < (blockInit[f] + 12); i++)
        {
          pixels.setPixelColor(i, PIXEL_RED);
        }
      }

      pixels.show();

      delay(1000);

      for (int f = 6; f < 9; f++)
      {
        for (int i = blockInit[f]; i < (blockInit[f] + 12); i++)
        {
          pixels.setPixelColor(i, PIXEL_CLEAR);
        }
      }

      for (int f = 15; f < 18; f++)
      {
        for (int i = blockInit[f]; i < (blockInit[f] + 12); i++)
        {
          pixels.setPixelColor(i, PIXEL_CLEAR);
        }
      }

      for (int f = 3; f < 6; f++)
      {
        for (int i = blockInit[f]; i < (blockInit[f] + 12); i++)
        {
          pixels.setPixelColor(i, PIXEL_YELLOW);
        }
      }

      for (int f = 12; f < 15; f++)
      {
        for (int i = blockInit[f]; i < (blockInit[f] + 12); i++)
        {
          pixels.setPixelColor(i, PIXEL_YELLOW);
        }
      }

      pixels.show();

      delay(1000);

      for (int f = 3; f < 6; f++)
      {
        for (int i = blockInit[f]; i < (blockInit[f] + 12); i++)
        {
          pixels.setPixelColor(i, PIXEL_CLEAR);
        }
      }

      for (int f = 12; f < 15; f++)
      {
        for (int i = blockInit[f]; i < (blockInit[f] + 12); i++)
        {
          pixels.setPixelColor(i, PIXEL_CLEAR);
        }
      }

      for (int f = 0; f < 3; f++)
      {
        for (int i = blockInit[f]; i < (blockInit[f] + 12); i++)
        {
          pixels.setPixelColor(i, PIXEL_GREEN);
        }
      }

      for (int f = 9; f < 12; f++)
      {
        for (int i = blockInit[f]; i < (blockInit[f] + 12); i++)
        {
          pixels.setPixelColor(i, PIXEL_GREEN);
        }
      }

      pixels.show();

      delay(1000);

      for (int f = 0; f < 3; f++)
      {
        for (int i = blockInit[f]; i < (blockInit[f] + 12); i++)
        {
          pixels.setPixelColor(i, PIXEL_CLEAR);
        }
      }

      for (int f = 9; f < 12; f++)
      {
        for (int i = blockInit[f]; i < (blockInit[f] + 12); i++)
        {
          pixels.setPixelColor(i, PIXEL_CLEAR);
        }
      }

      pixels.show();
    }

    for (int j = 0; j < 6; j++)
    {
      preVal[blockBias[j] + colCount - 1] = nowVal[blockBias[j] + colCount - 1]; //new preVal equals to nowVal
    }

    digitalWrite(gndCol[colCount], HIGH);

    proceed = 0;
  }

  if (proceed == 0)
  {
    if (colCount < 2)
    {
      colCount++;
    } else
    {
      colCount = 0;
    }

    proceed = 1;

  }
}


void logic()
{
  //For generating odd number, use 2n+1

  // 15 16 17
  // 12 13 14
  // 9  10 11
  //VS
  // 0  1  2
  // 3  4  5
  // 6  7  8

  for (int k = 0; k < 3; k++)
  {
    nearNum = (blockBias[k] + colCount - 1);
    farNum = nearNum + 9;

    if ((nowVal[nearNum] == 1) && (nowVal[farNum] == 1))
    {
      nextGame = 1;

      if ((preVal[nearNum] == 0) && (preVal[farNum] == 1))
      {
        //        Serial.print("Block ");
        //        Serial.print(farNum);
        //        Serial.print(" lose");

        nearScore++;

        for (int i = 0; i < NUMPIXELS; i++)
        {
          pixels.setPixelColor(i, PIXEL_CLEAR);
        }

        for (int i = blockInit[farNum]; i < (blockInit[farNum] + 12); i++)
        {
          pixels.setPixelColor(i, PIXEL_RED);
        }

        pixels.show();

        delay(200);

        for (int i = 108; i < NUMPIXELS; i++)
        {
          pixels.setPixelColor(i, PIXEL_RED);
        }

        pixels.show();

        while (digitalRead(startPin) == 1)
        {
          for (int i = 0; i < 108; i++)
          {
            pixels.setPixelColor(i, PIXEL_GREEN);
          }
          pixels.show();

          delay(400);

          for (int i = 0; i < 108; i++)
          {
            pixels.setPixelColor(i, PIXEL_CLEAR);
          }
          pixels.show();

          delay(400);
        }


      } else if ((preVal[nearNum] == 1) && (preVal[farNum] == 0))
      {
        //        Serial.print("Block ");
        //        Serial.print(nearNum);
        //        Serial.print(" lose");

        farScore++;

        for (int i = 0; i < NUMPIXELS; i++)
        {
          pixels.setPixelColor(i, PIXEL_CLEAR);
        }

        for (int i = blockInit[nearNum]; i < (blockInit[nearNum] + 12); i++)
        {
          pixels.setPixelColor(i, PIXEL_RED);
        }

        pixels.show();

        delay(200);

        for (int i = 0; i < 108; i++)
        {
          pixels.setPixelColor(i, PIXEL_RED);
        }

        pixels.show();

        while (digitalRead(startPin) == 1)
        {
          for (int i = 108; i < NUMPIXELS; i++)
          {
            pixels.setPixelColor(i, PIXEL_GREEN);
          }
          pixels.show();

          delay(400);

          for (int i = 108; i < NUMPIXELS; i++)
          {
            pixels.setPixelColor(i, PIXEL_CLEAR);
          }
          pixels.show();

          delay(400);
        }

      } else
      {
        //        Serial.println("Reached Impossible State.");
        while (1);
      }
    }
  }
}
