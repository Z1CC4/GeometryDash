#include "Player.h"


// Constructor and Methods

Player::Player(float startX, float startY, int w, int h) {
  x = startX;
  y = startY;
  width = w;
  height = h;
}

void Player::asSquare(Adafruit_SSD1306& oled) {
  oled.fillRect(x, y, width, height, SSD1306_WHITE);
}