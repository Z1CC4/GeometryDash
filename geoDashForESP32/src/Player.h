#pragma once
#include <Adafruit_SSD1306.h>

class Player {
public:
    float x;
    float y;
    int width;
    int height;

    Player(float startX, float startY, int w, int h);
    void asSquare(Adafruit_SSD1306& oled);
    void asCircle(Adafruit_SSD1306& oled);
    void asTriangle(Adafruit_SSD1306& oled);
};