//Ba Gua Zhen V0.2
//V0.1 Created by Yen Hau
//V0.2 Modified by Kaiyi:P

#include <RotaryEncoder.h>

// Setup a RoraryEncoder for pins A2 and A3:
RotaryEncoder encoder(A2, A3);

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#define PIN            10
#define NUMPIXELS      48
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int buttonPins[8] = {2, 3, 4, 5, 6, 7, 8, 9};
int ansPin[8] = {4, 8, 5, 7, 9, 3, 2, 6};
bool activation[8] = {0, 0, 0, 0, 0, 0, 0, 0};
bool newAct = false;
String characters[8] = {"li", "zhen", "kun", "gen", "kan", "xun", "qian", "dui"};
int pixelpos[8] = {0, 42, 36, 30, 24, 18, 12, 6};
int buttonState[8];
int errcheck = 0;
int fullcheck = 0;
//Buttons will use pin 2 to 9. And the pin 0 will pair with pin 4 and so on.

static int pos = 0;
int blockNum = 0;
static int block = buttonPins[blockNum];

void setup()
{
  Serial.begin(9600);
  Serial.println("Begin");
  pixels.begin();
  for (int i = 0; i < 8; i++)
  {
    pinMode(buttonPins[i], INPUT_PULLUP);
    buttonState[i] = 1; //Since it's input pullup, so use inverted logic
    errcheck += digitalRead(buttonPins[i]);
  }

  while (errcheck != 8)
  {
    //The BGZ haven't reset by the GM
    //Shining red light to warn
    for (int i = 0; i < NUMPIXELS; i++)
    {
      pixels.setPixelColor(i, pixels.Color(150, 0, 0));
    }
    pixels.show();

    delay(300);

    for (int i = 0; i < NUMPIXELS; i++)
    {
      pixels.setPixelColor(i, pixels.Color(0, 0, 0));
    }
    pixels.show();

    delay(300);

    errcheck = 0;
    for (int i = 0; i < 8; i++)
    {

      errcheck += digitalRead(buttonPins[i]);
    }
  }

  for (int i = 0; i < NUMPIXELS; i++)
  {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
  }
  pixels.show();

  for (int i = 0; i < NUMPIXELS; i++)
  {
    pixels.setPixelColor(i, pixels.Color(0, 150, 0));
  }
  pixels.show();
  delay(500);

  for (int i = 0; i < NUMPIXELS; i++)
  {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
  }
  pixels.show();

}

void loop()
{
  encoder.tick();
  int newPos = encoder.getPosition();

  if (pos - newPos >= 2) //Anti-clockwise Rotation
  {
    //    Serial.print(newPos);
    //Serial.println("Anti-clockwise rotated");
    pos = newPos;
    //Serial.println(pos);

    if(!activation[blockNum])
    {
      for (int i = pixelpos[blockNum]; i < (pixelpos[blockNum] + 6); i++)
    {
      pixels.setPixelColor(i, pixels.Color(0, 0, 0));
    }
    pixels.show();
    }
   
    blockNum ++;
    blockNum = ((blockNum > 7) ? 0 : blockNum);
    //Serial.println(blockNum);
    block = buttonPins[blockNum];
    Serial.println(characters[blockNum]);
  }
  else if (newPos - pos >= 2) //Clockwise Rotation
  {
    //    Serial.print(newPos);
    //Serial.println("Clockwise Rotation");
    pos = newPos;

    if(!activation[blockNum])
    {
      for (int i = pixelpos[blockNum]; i < (pixelpos[blockNum] + 6); i++)
    {
      pixels.setPixelColor(i, pixels.Color(0, 0, 0));
    }
    pixels.show();
    }

    blockNum --;
    blockNum = ((blockNum < 0) ? 7 : blockNum);
    //Serial.println(blockNum);
    block = buttonPins[blockNum];
    Serial.println(characters[blockNum]);
    //    Serial.println(pos);
    //    Serial.println("Choose block on the clockwise direction");
  }

  if (!activation[blockNum])
  {
    for (int i = pixelpos[blockNum]; i < (pixelpos[blockNum] + 6); i++)
    {
      pixels.setPixelColor(i, pixels.Color(150, 150, 0));
    }
    pixels.show();
  }

  //Check whether the corresponding button is being pressed to activate the blocks
  if (activation[blockNum])
  {
    if (digitalRead(ansPin[blockNum]) == 1)
    {
      activation[blockNum] = 0;

       for (int i = pixelpos[blockNum]; i < (pixelpos[blockNum] + 6); i++)
      {
        pixels.setPixelColor(i, pixels.Color(150, 150, 0));
      }
      pixels.show();
    }
  }
  else
  {
    if (digitalRead(ansPin[blockNum]) == 0)
    {
      activation[blockNum] = 1;

      for (int i = pixelpos[blockNum]; i < (pixelpos[blockNum] + 6); i++)
      {
        pixels.setPixelColor(i, pixels.Color(0, 0, 150));
      }
      pixels.show();

    }
  }

  int sum = 0;
  for (int count = 0; count < 8; count ++)
  {
    sum += activation[count];
  }

  if (sum == 8)
  {
    Serial.println("GAME WIN");

    while(1)
    {
      for (int i = 0; i < NUMPIXELS; i++)
      {
        pixels.setPixelColor(i, pixels.Color(0, 0, 150));
      }
      pixels.show();

      delay(300);
      for (int i = 0; i < NUMPIXELS; i++)
      {
        pixels.setPixelColor(i, pixels.Color(0, 0, 0));
      }
      pixels.show();

      delay(300);
    }
  }

}
