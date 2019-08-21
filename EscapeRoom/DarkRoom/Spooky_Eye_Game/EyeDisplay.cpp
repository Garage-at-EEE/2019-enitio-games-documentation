#include "EyeDisplay.h"

EyeDisplay::EyeDisplay(uint8_t rst_pin) :
  display(screenWidth, screenHeight, &Wire, rst_pin) {}

void EyeDisplay::begin() {
  // Begin. Read address is 0x3C.
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
//  display.SSD1306_command(0x81);
//  display.SSD1306_command(0x0F);
}

void EyeDisplay::update() {
  // Check if current pos exceeded final pos
  updateMove(iris_x, iris_xf, iris_dx);
  updateMove(iris_y, iris_yf, iris_dy);
  updateMove(top_d, top_df, top_dd);

  // Blink update
  if (isBlinking) {
    if (top_d == top_d0) {
      isBlinking = false;
    }
    if (top_d == btm_d0) {
      moveTop(top_d0);
    }
  }

  draw();
}

void EyeDisplay::clear() {
  display.clearDisplay();
  display.display();
}

void EyeDisplay::moveIris(float x, float y) {
  iris_xf = x;
  iris_yf = y;

  float delta_x = iris_xf - iris_x;
  float delta_y = iris_yf - iris_y;
  float distance = sqrt(delta_x * delta_x + delta_y * delta_y);

  iris_dx = (distance == 0) ? 0 : delta_x / distance * iris_v;
  iris_dy = (distance == 0) ? 0 : delta_y / distance * iris_v;
}

void EyeDisplay::moveTop(float d) {
  top_df = d;

  top_dd = (top_df - top_d >= 0) ? top_v : - top_v;
}

void EyeDisplay::setIrisSpeed(float v) {
  iris_dx *= v / iris_v;
  iris_dy *= v / iris_v;

  iris_v = v;
}

void EyeDisplay::setTopSpeed(float v) {
  top_dd *= v / top_v;

  top_v = v;
}

bool EyeDisplay::isIrisMoving() {
  return iris_dy != 0 && iris_dx != 0;
}

bool EyeDisplay::isTopMoving() {
  return top_dd != 0;
}

void EyeDisplay::blink() {
  isBlinking = true;
  moveTop(btm_d0);
}

void EyeDisplay::updateMove(float &x, float &xf, float &dx) {
  x += dx;
  if ((dx >= 0 ? 1 : -1) * (xf - x) < 0) {
    dx = 0;
    x = xf;
  }
}



/*-- Drawing functions --*/

void EyeDisplay::draw() {
  display.clearDisplay();

  // Draw iris + pupil
  drawIris(iris_x, iris_y);
  drawPupil(iris_x, iris_y);

  // Remove overlap
  rmOverlapLid(center_x, center_y + 4, lidLength, top_d, true);
  rmOverlapLid(center_x, center_y - 1, lidLength, btm_d0, false);

  drawLid(center_x, center_y, lidLength, top_d, 4);
  drawLid(center_x, center_y, lidLength, btm_d0, 3);

  display.display();
}

void EyeDisplay::drawIris(int off_x, int off_y) {
  double d = sqrt(off_x * off_x + off_y * off_y);
  int x = iris_x0 + off_x;
  int y = iris_y0 + off_y;
  int r = iris_r0 - d / 10;

  display.fillCircle(x, y, r, WHITE);
}

void EyeDisplay::drawPupil(int off_x, int off_y) {
  double d = sqrt(off_x * off_x + off_y * off_y);
  int x = iris_x0 + off_x * 1.2;
  int y = iris_y0 + off_y * 1.2;
  int r = pupil_r0 - d / 20;

  display.fillCircle(x, y, r, BLACK);
}

// drawLid() algorithm is very similar to writeCircle() from the gfx library
// Rewritten to support stacking of the same circle, and to skip some section of the circle
void EyeDisplay::drawLid(int x0, int yp, int c, int d, uint16_t stack) {
  // Special case d=0, draw straight line instead of arc
  if (d == 0) {
    display.writeFillRect(x0 - c, yp, 2 * c + 1, stack, WHITE);
    return;
  }


  // All variables below are constants, declared here to prevent recalculation
  int y0 = yp + (d * d - c * c) / (2 * d);
  int r = abs((d * d + c * c) / (2 * d));
  int maxY = r - abs(d);
  bool isTop = d < 0;


  // Used for circle generation
  // x and y needs to be long, int will cause overflow
  int f = 1 - r;
  int ddF_x = 1;
  int ddF_y = -2 * r;
  long x = 0;
  long y = r;

  // Draw start pixel
  int ys = (isTop) ? y0 - r : y0 + r;
  display.drawFastVLine(x0, ys, stack, WHITE);

  // Continue when x / y < maxX / maxY, maxX is c
  // Rearranged below to keep integer computation
  while (maxY * x < c * y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    int yc = (isTop) ? y0 - y : y0 + y;
    display.drawFastVLine(x0 + x, yc, stack, WHITE);
    display.drawFastVLine(x0 - x, yc, stack, WHITE);
  }
}

void EyeDisplay::rmOverlapPixel(int x, int y, bool rmUp) {
  if (rmUp) {
    display.writeFastVLine(x, 0, y + 1, BLACK);
  } else {
    display.writeFastVLine(x, y, screenHeight - y, BLACK);
  }
}

void EyeDisplay::rmOverlapLid(int x0, int yp, int c, int d, bool rmUp) {
  // Special case d=0
  if (d == 0) {
    if (rmUp) {
      display.writeFillRect(x0 - c, 0, 2 * c + 1, yp + 1, BLACK);
    } else {
      display.writeFillRect(x0 - c, yp, 2 * c + 1, screenHeight - yp, BLACK);
    }
    return;
  }


  // All variables below are constants, declared here to prevent recalculation
  int y0 = yp + (d * d - c * c) / (2 * d);
  int r = abs((d * d + c * c) / (2 * d));
  int maxY = r - abs(d);
  bool isTop = d < 0;


  // Used for circle generation
  // x and y needs to be long, int will cause overflow
  int f = 1 - r;
  int ddF_x = 1;
  int ddF_y = -2 * r;
  long x = 0;
  long y = r;

  // Remove overlap over starting pixel
  int ys = (isTop) ? y0 - r : y0 + r;
  rmOverlapPixel(x0, ys, rmUp);

  // Continue when x / y < maxX / maxY, maxX is c
  // Rearranged below to keep integer computation
  while (maxY * x < c * y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    int yc = (isTop) ? y0 - y : y0 + y;
    rmOverlapPixel(x0 + x, yc, rmUp);
    rmOverlapPixel(x0 - x, yc, rmUp);
  }
}
