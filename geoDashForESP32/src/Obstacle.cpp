#include "Obstacle.h"

Obstacle::Obstacle(float startX, float startY, int w, int h) {
    x = startX;
    y = startY;
    width = w;
    height = h;
  }

  void Obstacle::asSquare(Adafruit_SSD1306& oled, int groundY) {
    oled.fillRect(x, groundY - height, width, height, SSD1306_WHITE);
  };

 

void Obstacle::updatePosition(float speed) {
    if ((x + width > 0)) {
      x -= speed;
    }
  }