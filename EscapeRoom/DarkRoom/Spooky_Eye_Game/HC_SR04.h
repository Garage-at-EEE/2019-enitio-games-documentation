#ifndef HC_SR04_H
#define HC_SR04_H

#include <Arduino.h>

#define TIMEOUT     18000    // In microseconds.
#define TIMEOUT_VAL 1000     // If timeout occurs, distance() method returns this value.

class HC_SR04
{
  public:

    HC_SR04(uint8_t assign_trigPin, uint8_t assign_echoPin);
    float distance();


  private:

    uint8_t  trigPin;
    uint8_t  echoPin;
    uint32_t echoDuration;
    void     resetEchoPin();
};

#endif
