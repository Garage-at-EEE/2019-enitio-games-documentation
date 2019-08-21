#include "HC_SR04.h"


HC_SR04::HC_SR04(uint8_t assign_trigPin, uint8_t assign_echoPin)
{
  trigPin = assign_trigPin;
  echoPin = assign_echoPin;

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  digitalWrite(trigPin, LOW);
}


float HC_SR04::distance()
{
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(100);
  digitalWrite(trigPin, LOW);

  echoDuration = pulseIn(echoPin, HIGH, TIMEOUT);  // Measure the pulse length of the echo pin, using timeout.
  Serial.println(echoDuration);
  while (echoDuration <= 0) {
    resetEchoPin();
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(100);
    digitalWrite(trigPin, LOW);
    echoDuration = pulseIn(echoPin, HIGH, TIMEOUT);  // Measure the pulse length of the echo pin, using timeout.
  }

  //
  //  if (echoDuration == 0)
  //    return TIMEOUT_VAL;

  return 0.017 * echoDuration;      // Convert the echo time (us) into distance (cm).
  // Assuming sound propagation speed of 340 m/s.
}

void HC_SR04::resetEchoPin() {
  pinMode(echoPin, OUTPUT);
  delayMicroseconds(10);
  digitalWrite(echoPin, LOW);
  delayMicroseconds(10);
  pinMode(echoPin, INPUT);
  delayMicroseconds(10);
  if (Serial) {
//    Serial.println("resetting echo pin");
  }
}
