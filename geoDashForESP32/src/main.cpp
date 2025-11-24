#include "Obstacle.h"
#include "Player.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <vector>

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
const int rightButtonPin = 12;

float obstacleSpeed = 2.0;
bool play = true;
int score = 0;
int highscore = 0;
int spawnInterval = 2000;
int lastSpawnTime = 0;
int lastMilestone = 0;

std::vector<Obstacle> obstacles;

int selectedShape = 0; // 0=Square, 1=Circle, 2=Triangle

enum GameState { MENU, PLAYING, GAME_OVER };
GameState currentState = MENU;

float velocityY = 0;
bool isJumping = false;

float obstacleX = SCREEN_WIDTH;

Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Global instances
Player player(playerXPosition, groundY - playerHeight, playerWidth,
              playerHeight);
Obstacle obstacle(obstacleX, groundY, 10, 20);

void reset() {
  score = 0;
  obstacle.x = SCREEN_WIDTH;
  player.y = groundY - playerHeight;
  isJumping = false;
  velocityY = 0;
  obstacles.clear();
  lastSpawnTime = millis();
  spawnInterval = 2000;
  lastMilestone = 0;
  obstacleSpeed = 2.0;
}

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
  oled.setCursor(0, 28);
  oled.print("Score: ");
  oled.println(score);
  oled.setCursor(0, 38);
  oled.print("Best: ");
  oled.println(highscore);

  // Select shape with left/right
  oled.setTextSize(1);
  oled.setCursor(5, 50);
  oled.println("< Menu    Restart >");
  oled.display();

  // Left Button - previous Shape
  if (digitalRead(leftButtonPin) == LOW) {
    delay(200);
    reset();
    currentState = MENU;
  }

  // Right Button - next Shape
  if (digitalRead(rightButtonPin) == LOW) {
    delay(200);
    reset();
    currentState = PLAYING;
  }

  // Start Button - start game
  if (digitalRead(gameButtonPin) == LOW) {
    delay(200);
    reset();
    currentState = PLAYING;
  }
}

// game helper functions
void ground() { oled.drawLine(0, 54, 127, 54, SSD1306_WHITE); }

void updatePlayer() {
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
  // simply AABB collision detection
  if (playerXPosition < obstacle.x + obstacle.width &&
      playerXPosition + playerWidth > obstacle.x &&
      player.y < groundY - obstacle.height + obstacle.height &&
      player.y + playerHeight > groundY - obstacle.height) {
    // Collision detected
    return true;
  }
  return false;
}

// start Jump
void startJump() {
  if (!isJumping) {
    isJumping = true;
    velocityY = -5.0;
  }
}

void spawnObstacle(int score) {
  if (score % 10 == 0 && score != 0 && score != lastMilestone) {
    spawnInterval -= (score / 10) * 100; // every 10 points 100ms less
    obstacleSpeed += 0.2;
    lastMilestone = score;
  }
  // min spawn interval
  if (spawnInterval < 800) {
    spawnInterval = 800;
  }
  // max obstacle speed
  if (obstacleSpeed > 5.0) {
    obstacleSpeed = 5.0;
  }

  if (millis() > lastSpawnTime + spawnInterval) {
    lastSpawnTime = millis();
    int obstacleHeight = random(5, 30);
    int obstacleWidth = random(5, 15);
    lastSpawnTime = millis();
    obstacles.push_back(Obstacle(SCREEN_WIDTH, groundY - obstacleHeight,
                                 obstacleWidth, obstacleHeight));
  }
}

void displayScore() {
  oled.setTextSize(0.5);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(0, 0);
  oled.print("Score: ");
  oled.println(score);
}

void game(int playerShape) {
  oled.clearDisplay();
  // Button check for jump
  if (digitalRead(gameButtonPin) == LOW) {
    startJump();
  }

  updatePlayer();

  spawnObstacle(score);

  // Loop backwards, otherwise would skip elements when erasing:
  // Vector: [A, B, C, D]
  // i=0: Check A, delete A → [B, C, D]
  // i=1: Check now C (B was skipped!) → [B, C, D]
  // because B is now at position 0, but i is already at 1!

  for (int i = obstacles.size() - 1; i >= 0; i--) {
    Obstacle &obstacle = obstacles[i];
    if (checkCollision(obstacle, player)) {
      currentState = GAME_OVER;
      return;
    }
    obstacle.updatePosition(obstacleSpeed);
    obstacle.asSquare(oled, groundY);

    if (obstacle.x + obstacle.width < 1) {
      obstacles.erase(obstacles.begin() + i);
      score++;
    }
  }

  // Show Ground
  ground();

  // Show Player
  if (playerShape == 0)
    player.asSquare(oled);
  else if (playerShape == 1)
    player.asCircle(oled);
  else if (playerShape == 2)
    player.asTriangle(oled);

  displayScore();

  oled.display();
  // Control frame rate - 1000ms ÷ 60 = 16.666...ms -> something around 30-60
  // FPS, code takes some time to execute
  delay(16);
}

void menuScreen() {
  oled.clearDisplay();
  oled.setTextSize(2);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(10, 10);
  oled.println("GeoDash");

  // Select shape left right
  oled.setTextSize(1);
  oled.setCursor(20, 35);
  oled.print("Shape: ");
  if (selectedShape == 0)
    oled.println("Square");
  else if (selectedShape == 1)
    oled.println("Circle");
  else if (selectedShape == 2)
    oled.println("Triangle");

  oled.setCursor(5, 50);
  oled.println("< Change    Start >");
  oled.display();

  // Left Button - previous shape
  if (digitalRead(leftButtonPin) == LOW) {
    delay(200);
    selectedShape--;
    if (selectedShape < 0)
      selectedShape = 2; // Wrap around
  }

  // Right Button - next shape
  if (digitalRead(rightButtonPin) == LOW) {
    delay(200);
    currentState = PLAYING;
  }

  // Start Button - start game
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
    menuScreen();
    break;
  case PLAYING:
    game(selectedShape);
    break;
  case GAME_OVER:
    gameOverScreen();
    break;
  }
}
