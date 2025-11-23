#include "Player.h"

// Constructor and Methods

Player::Player(float startX, float startY, int w, int h) {
  x = startX;
  y = startY;
  width = w;
  height = h;
}

void Player::asSquare(Adafruit_SSD1306 &oled) {
  oled.fillRect(x, y, width, height, SSD1306_WHITE);
}

void Player::asCircle(Adafruit_SSD1306 &oled) {
  int radius = width / 2;
  oled.fillCircle(x + radius, y + radius, radius, SSD1306_WHITE);
}

void Player::asTriangle(Adafruit_SSD1306 &oled) {
  // Dreieck mit Spitze nach oben
  int x0 = x + width / 2; // Spitze oben (Mitte)
  int y0 = y;             // Oben
  int x1 = x;             // Links unten
  int y1 = y + height;
  int x2 = x + width; // Rechts unten
  int y2 = y + height;

  oled.fillTriangle(x0, y0, x1, y1, x2, y2, SSD1306_WHITE);
}