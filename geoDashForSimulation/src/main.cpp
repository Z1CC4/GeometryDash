#include "Obstacle.h"
#include "Player.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

const int groundY = 54;
const int playerWidth = 10;
const int playerHeight = 10;
const int playerXPosition = 20;
const int gameButtonPin = 14;
const int leftButtonPin = 27;
const int rightButtonPin = 26;
const float obstacleSpeed = 2.0;

bool play = true;
int score = 0;
int highscore = 0;

// Game States
enum GameState { MENU, PLAYING, GAME_OVER };
GameState currentState = MENU;

float velocityY = 0;
bool isJumping = false;

float obstacleX = SCREEN_WIDTH;

Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Globale Instanzen
Player player(playerXPosition, groundY - playerHeight, playerWidth,
              playerHeight);
Obstacle obstacle(obstacleX, groundY, 10, 20);

void gameOverScreen() {
  if (score > highscore) {
    highscore = score;
  }
  oled.clearDisplay();
  oled.setTextSize(2);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(20, 10);
  oled.println("Game Over");
  oled.setTextSize(1);
  oled.setCursor(0, 30);
  oled.print("Score: ");
  oled.println(score);
  oled.setCursor(0, 45);
  oled.print("Best: ");
  oled.println(highscore);
  oled.display();

  if (digitalRead(gameButtonPin) == LOW) {
    delay(200); 
    score = 0;
    obstacle.x = SCREEN_WIDTH;
    player.y = groundY - playerHeight;
    isJumping = false;
    velocityY = 0;
    currentState = PLAYING;
  }
}

// game helper functions
void ground() { oled.drawLine(0, 54, 127, 54, SSD1306_WHITE); }

void updatePlayer(Player player) {
  if (isJumping) {
    player.y += velocityY;
    velocityY += 0.3; // Gravitation

    if (player.y >= groundY - playerHeight) {
      player.y = groundY - playerHeight;
      velocityY = 0;
      isJumping = false;
    }
  }
}

bool checkCollision(Obstacle obstacle, Player player) {
  // Einfache AABB-Kollisionsabfrage
  if (playerXPosition < obstacle.x + obstacle.width &&
      playerXPosition + playerWidth > obstacle.x &&
      player.y < groundY - obstacle.height + obstacle.height &&
      player.y + playerHeight > groundY - obstacle.height) {
    // Kollision erkannt
    return true;
  }
  return false;
}

// Sprung-Funktionen
void startJump() {
  if (!isJumping) {
    isJumping = true;
    velocityY = -5.0;
  }
}

void game(int playerShape) {
  oled.clearDisplay();

  // Button-Check für Sprung
  if (digitalRead(gameButtonPin) == LOW) {
    startJump();
  }

  // Player-Physik updaten
  updatePlayer(player);

  score += obstacle.updatePosition(obstacleSpeed, SCREEN_WIDTH);

  if (checkCollision(obstacle, player)) {
    currentState = GAME_OVER;
    return;
  }

  // Boden zeichnen
  ground();

  // Player zeichnen
  player.asSquare(oled);

  obstacle.asSquare(oled, groundY);

  oled.display();
  delay(16);
}

void startingScreen() {
  oled.clearDisplay();
  oled.setTextSize(2);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(10, 10);
  oled.println("GeoDash");
  oled.setTextSize(1);
  oled.setCursor(5, 40);
  oled.println("Press to Start");
  oled.display();

  if (digitalRead(gameButtonPin) == LOW) {
    delay(200); 
    currentState = PLAYING;
  }
}

// setup
void setup() {
  Serial.begin(9600);
  if (!oled.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
  }
  pinMode(gameButtonPin, INPUT_PULLUP);
  pinMode(leftButtonPin, INPUT_PULLUP);
  pinMode(rightButtonPin, INPUT_PULLUP);
}

void loop() {
  switch (currentState) {
  case MENU:
    startingScreen();
    break;
  case PLAYING:
    game(0);
    break;
  case GAME_OVER:
    gameOverScreen();
    break;
  }
}
