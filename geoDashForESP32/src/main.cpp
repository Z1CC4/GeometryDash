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

int selectedShape = 0; // 0=Quadrat, 1=Kreis, 2=Dreieck

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

  // Shape auswählen mit links/rechts
  oled.setTextSize(1);
  oled.setCursor(5, 50);
  oled.println("< Menu    Restart >");
  oled.display();

  // Links Button - vorherige Shape
  if (digitalRead(leftButtonPin) == LOW) {
    delay(200); // Entprellen
    reset();
    currentState = MENU;
  }

  // Rechts Button - nächste Shape
  if (digitalRead(rightButtonPin) == LOW) {
    delay(200); // Entprellen
    reset();
    currentState = PLAYING;
  }

  // Start Button - Spiel starten
  if (digitalRead(gameButtonPin) == LOW) {
    delay(200);
    reset();
    currentState = PLAYING;
  }
}

// game helper functions
void ground() { oled.drawLine(0, 54, 127, 54, SSD1306_WHITE); }

void updatePlayer(Player &player) {
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

// Sprung-Funktionen
void startJump() {
  if (!isJumping) {
    isJumping = true;
    velocityY = -5.0;
  }
}

void spawnObstacle(int score) {
  // spwan dauer verkürzen bei jeden 10 punkten
  if (score % 10 == 0 && score != 0 &&
      score != lastMilestone) {          // change only every 10 points
    spawnInterval -= (score / 10) * 100; // every 10 points 100ms less
    obstacleSpeed += 0.2;
    lastMilestone = score;
  }
  if (spawnInterval < 800) {
    spawnInterval = 800; // min spawn interval
  }
  if (obstacleSpeed > 5.0) {
    obstacleSpeed = 5.0; // max speed
  }

  // einen timer, wenn der abgelaufen ist wird ein neues Hindernis gespawnt
  if (millis() > lastSpawnTime + spawnInterval) {
    // hindernisse müssen höhe und breite zufällig kommen, position auch - muss
    // alles ein min und max geben
    lastSpawnTime = millis();
    int obstacleHeight = random(5, 30);
    int obstacleWidth = random(5, 15);
    int obsticleY = random(10, groundY - obstacleHeight);
    lastSpawnTime = millis();
    obstacles.push_back(
        Obstacle(SCREEN_WIDTH, obsticleY, obstacleWidth, obstacleHeight));
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

  updatePlayer(player);

  spawnObstacle(score);

  // Loop backwards, otherwise would skip elements when erasing:
  // Vector: [A, B, C, D]
  // i=0: Check A, lösche A → [B, C, D]
  // i=1: Check jetzt C (B wurde übersprungen!) → [B, C, D]
  // weil B jetzt an Position 0 ist, aber i ist schon bei 1!

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

  // Boden zeichnen
  ground();

  // Player zeichnen
  if (playerShape == 0)
    player.asSquare(oled);
  else if (playerShape == 1)
    player.asCircle(oled);
  else if (playerShape == 2)
    player.asTriangle(oled);

  displayScore();

  oled.display();
  delay(16);
}

void menuScreen() {
  oled.clearDisplay();
  oled.setTextSize(2);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(10, 10);
  oled.println("GeoDash");

  // Shape auswählen mit links/rechts
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

  // Links Button - vorherige Shape
  if (digitalRead(leftButtonPin) == LOW) {
    delay(200); // Entprellen
    selectedShape--;
    if (selectedShape < 0)
      selectedShape = 2; // Wrap around
  }

  // Rechts Button - nächste Shape
  if (digitalRead(rightButtonPin) == LOW) {
    delay(200); // Entprellen
    currentState = PLAYING;
  }

  // Start Button - Spiel starten
  if (digitalRead(gameButtonPin) == LOW) {
    delay(200);
    currentState = PLAYING;
  }
}

// setup
void setup() {
  Serial.begin(9600);
  Wire.begin(22, 20); 
  Wire.setClock(400000);
  delay(200);
  
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  delay(100);
  Wire.beginTransmission(0x3C);
  if(Wire.endTransmission() != 0) {
    oled.begin(SSD1306_SWITCHCAPVCC, 0x3D);
    delay(100);
  }
  
  oled.clearDisplay();
  oled.display();
  delay(200);
  
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
