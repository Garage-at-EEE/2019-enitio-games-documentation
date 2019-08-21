#include <Button.h>
#include <Adafruit_NeoPixel.h>
#include <Servo.h>

#define PXL 2
#define PXL_COUNT 5

int pos = 0;
int coinRe = 0;
unsigned long previousMillis = 0;
bool up = true;
int delayCount = 0;

Servo myservo1;
Servo myservo2;
Servo myservo3;
Servo myservo4;

Button button1(9);
Button button2(10);
Button button3(11);
Button button4(12);
Button button5(13);


Adafruit_NeoPixel strip(PXL_COUNT, PXL, NEO_GRB + NEO_KHZ800);
bool case1 = false;
bool case2 = false;
bool case3 = false;
bool case4 = false;
bool case5 = false;


void setup() {
  // put your setup code here, to run once:
  strip.begin();
  strip.show();
  pinMode(9, INPUT);
  pinMode(10, INPUT);
  pinMode(11, INPUT);
  pinMode(12, INPUT);
  pinMode(13, INPUT);
  Serial.begin(9600);
  myservo1.attach(3);
  myservo2.attach(4);
  myservo3.attach(5);
  myservo4.attach(6);
  button1.begin();
  button2.begin();
  button3.begin();
  button4.begin();
  button5.begin();
  myservo4.write(coinRe);
}

void loop() {
  // put your main code here, to run repeatedly:
  unsigned long currentMillis = millis();
  if (up && currentMillis-previousMillis > 15) {
    pos++;
    myservo1.write(pos);
    myservo2.write(pos);
    myservo3.write(pos);
    previousMillis = currentMillis;
    if (pos == 90) {
      up = !up;
    }
  } else if (!up && currentMillis-previousMillis > 15) {
    pos--;
    myservo1.write(pos);
    myservo2.write(pos);
    myservo3.write(pos);
    previousMillis = currentMillis;
    if (pos == 0) {
      up = !up;
    }
  }

  if (delayCount<20){
    case1 = false;
    case2 = false;
    case3 = false;
    case4 = false;
    case5 = false;
    delayCount++;
  }
  
  Serial.println(digitalRead(9));
  if (button1.pressed()) {
    case5 = !case5;
  } else if(button2.pressed()) {
    case4 = !case4;
  } else if(button3.pressed()) {
    case3 = !case3;
  } else if(button4.pressed()) {
    case2 = !case2;
  } else if(button5.pressed()) {
    case1 = !case1;
  }

  if (case1) {
    strip.setPixelColor( 0, 255, 0, 0);
    strip.show();
  } else {
    strip.setPixelColor( 0, 0, 0, 0);
    strip.show();
  }

  if (case2) {
    strip.setPixelColor( 1, 0, 0, 255);
    strip.show();
  } else {
    strip.setPixelColor( 1, 0, 0, 0);
    strip.show();
  }

  if (case3) {
    strip.setPixelColor( 2, 255, 0, 0);
    strip.show();
  } else {
    strip.setPixelColor( 2, 0, 0, 0);
    strip.show();
  }

  if (case4) {
    strip.setPixelColor( 3, 0, 0, 255);
    strip.show();
  } else {
    strip.setPixelColor( 3, 0, 0, 0);
    strip.show();
  }

  if (case5) {
    strip.setPixelColor( 4, 255, 0, 0);
    strip.show();
  } else {
    strip.setPixelColor( 4, 0, 0, 0);
    strip.show();
  }

  if (case1 && !case2 && case3 && !case4 && case5) {
    for (int coinRe = 0; coinRe <= 90; coinRe += 1) {
    myservo4.write(coinRe);
    delay(15);
    }
    for(;;);
  }

  
}



//strip.setPixelColor( 4, 255, 0, 0);
//    strip.show();
//strip.setPixelColor( 3, 255, 0, 0);
//    strip.show();
//strip.setPixelColor( 2, 255, 0, 0);
//    strip.show();
//strip.setPixelColor( 1, 255, 0, 0);
//    strip.show();
//strip.setPixelColor( 0, 255, 0, 0);
//    strip.show();
//strip.clear();
//  strip.show();
