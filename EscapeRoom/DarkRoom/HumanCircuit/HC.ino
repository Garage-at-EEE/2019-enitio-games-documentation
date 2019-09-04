//Human Circuit V0.1
//Created by Yen Hau

int buttonPin = 10;

void setup() {

  pinMode(buttonPin, INPUT_PULLUP);

  pixels.begin();

  Serial.begin(115200);
  Serial.println("Ready");
}

void loop() {

  if (digitalRead(buttonPin) == 0)
  {
    Serial.println("Neopixel Activated");
    //The painting is activated
    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(200, 200, 200));
    }

    pixels.show();

  } else
  {
    //Turn off the neopixels
    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 0, 0));
      pixels.show();
    }
  }
} // Loop
