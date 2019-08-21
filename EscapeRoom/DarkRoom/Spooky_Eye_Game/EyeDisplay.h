#ifndef EYE_DISPLAY_H
#define EYE_DISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class EyeDisplay {
  public:
    EyeDisplay(uint8_t rst_pin);
    void begin();
    void update();
    void clear();

    void blink();
    void moveIris(float x, float y);
    void moveTop(float d);

    void setIrisSpeed(float v);
    void setTopSpeed(float v);

    bool isIrisMoving();
    bool isTopMoving();

    static constexpr uint16_t
    screenWidth  = 128,
    screenHeight = 64;

    static constexpr int
    center_x = screenWidth / 2,
    center_y = screenHeight / 2;

    // Initial eye lid values
    static constexpr int
    lidLength = 60,
    top_d0 = -28,
    btm_d0 = 28;

    // Initial iris values
    static constexpr int
    iris_x0 = center_x,
    iris_y0 = center_y,
    iris_r0 = 30,
    pupil_r0 = 13;


  private:
    Adafruit_SSD1306 display;

    // Eye lids current pos
    bool isBlinking = false;

    float top_d  = top_d0;
    float top_df = top_d0;
    float top_dd = 0;

    float top_v = 3;

    // Iris current pos
    float iris_x  = 0;
    float iris_y  = 0;
    float iris_xf = 0;
    float iris_yf = 0;
    float iris_dx = 0;
    float iris_dy = 0;

    float iris_v = 1;

    void updateMove(float &x, float &xf, float &dx);

    void draw();
    void drawIris(int off_x, int off_y);
    void drawPupil(int off_x, int off_y);
    void drawLid(int x0, int yp, int c, int d, uint16_t stack);
    void rmOverlapPixel(int x, int y, bool rmUp);
    void rmOverlapLid(int x0, int yp, int c, int d, bool rmUp);
};

#endif
