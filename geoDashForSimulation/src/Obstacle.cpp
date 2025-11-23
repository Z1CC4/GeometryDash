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

 

int Obstacle::updatePosition(float speed, int SCREEN_WIDTH) {
    // Wenn komplett links raus, respawn rechts
    if ((x + width > 0)) {
      x -= speed;
      return 0;
    } else {
      x = SCREEN_WIDTH;
      return 1;
      // Eleminate obstacle
    }
  }