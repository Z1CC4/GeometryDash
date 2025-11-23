#pragma once
#include <Adafruit_SSD1306.h>

class Obstacle {
public:
  float x;
  float y;
  int width;
  int height;

public:
  Obstacle(float startX, float startY, int w, int h);
  void asSquare(Adafruit_SSD1306 &oled, int groundY);
  void updatePosition(float speed);
};