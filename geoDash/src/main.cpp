#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

#define SDA_PIN 22
#define SCL_PIN 20

#define BUTTON_JUMP 14
#define BUTTON_DOWN 12

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

uint8_t displayAddress = 0x3C;
bool displayFound = false;

struct Player {
  float x;
  float y;
  float velocityY;
  bool isJumping;
  bool onPlatform;
  float platformY;
  const int width = 8;
  const int height = 8;
  const int groundY = SCREEN_HEIGHT - 10;
};

enum ObstacleType {
  OBSTACLE_LOW,
  OBSTACLE_HIGH,
  OBSTACLE_MEDIUM,
  OBSTACLE_VERY_HIGH,
  OBSTACLE_FLYING,
  OBSTACLE_GAP,
  OBSTACLE_WIDE,
  OBSTACLE_NARROW,
  OBSTACLE_ULTRA_HIGH,
  OBSTACLE_PLATFORM,
  OBSTACLE_TRAMPOLINE,
  OBSTACLE_DUCK
};

struct Obstacle {
  float x;
  int height;
  int width;
  bool active;
  ObstacleType type;
  int gapY;
  int gapHeight;
  int gapX;
};

Player player;
Obstacle obstacles[5];
int score = 0;
int gameSpeed = 2;
bool gameOver = false;
unsigned long lastObstacleTime = 0;
unsigned long lastFrameTime = 0;
const int obstacleSpawnInterval = 1500;

int patternSeed = 0;
int nextObstacleHeight = 0;
int nextObstacleWidth = 6;

void resetGame();
void spawnObstacle();
void generateObstaclePattern(int index, int patternType = -1);
bool checkCollision(Player p, Obstacle o);
void drawGame();
void scanI2C();

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  while (!Serial && millis() < 3000) {
    delay(10);
  }
  
  Serial.println(F("\n\n=== DISPLAY DIAGNOSTICS ==="));
  Serial.println(F("Adafruit Feather ESP32 V2"));
  Serial.println(F("Connections:"));
  Serial.println(F("  Display 'data' -> Feather pin SDA (GPIO 22)"));
  Serial.println(F("  Display 'CLK'  -> Feather pin SCL (GPIO 20)"));
  Serial.println(F("  Display VCC    -> Feather 3V (or VIN if 3.3V)"));
  Serial.println(F("  Display GND    -> Feather GND"));
  Serial.print(F("\nI2C pins configured: SDA=GPIO "));
  Serial.print(SDA_PIN);
  Serial.print(F(", SCL=GPIO "));
  Serial.println(SCL_PIN);
  
  Serial.println(F("Initializing I2C..."));
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);
  delay(200);
  
  Serial.println(F("\nScanning I2C bus..."));
  Serial.println(F("If you see '0x3C' or '0x3D', the display is connected!"));
  scanI2C();
  delay(2000);
  
  Serial.println(F("\n=== DISPLAY INITIALIZATION ATTEMPT ==="));
  Serial.println(F("Display: SSD1306 OLED"));
  
  displayFound = false;
  
  Serial.print(F("Trying address 0x3C..."));
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  delay(100);
  Wire.beginTransmission(0x3C);
  byte error = Wire.endTransmission();
  if(error == 0) {
    Serial.println(F(" OK! Display found at 0x3C"));
    displayFound = true;
    displayAddress = 0x3C;
  } else {
    Serial.println(F(" Failed"));
    Serial.print(F("Trying address 0x3D..."));
    display.begin(SSD1306_SWITCHCAPVCC, 0x3D);
    delay(100);
    Wire.beginTransmission(0x3D);
    error = Wire.endTransmission();
    if(error == 0) {
      Serial.println(F(" OK! Display found at 0x3D"));
      displayFound = true;
      displayAddress = 0x3D;
    } else {
      Serial.println(F(" Failed"));
      Serial.println(F("\nTrying alternative initialization..."));
      if(display.begin(SSD1306_SWITCHCAPVCC)) {
        Serial.println(F(" OK! Display found (auto-detect)"));
        displayFound = true;
        displayAddress = 0x3C;
      } else {
        Serial.println(F(" Failed even with auto-detect"));
      }
    }
  }
  
  if(!displayFound) {
    Serial.println(F("\n!!! ERROR: Display not found !!!"));
    Serial.println(F("Check:"));
    Serial.println(F("1. I2C connections:"));
    Serial.print(F("   SDA -> GPIO "));
    Serial.println(SDA_PIN);
    Serial.print(F("   SCL -> GPIO "));
    Serial.println(SCL_PIN);
    Serial.println(F("2. Power: VCC -> 3.3V, GND -> GND"));
    Serial.println(F("3. Display powered on (some have a switch)"));
    Serial.println(F("\nProgram will continue but display won't work."));
    delay(3000);
  } else {
    Serial.println(F("\nDisplay initialized successfully!"));
    
    display.clearDisplay();
    display.display();
    delay(200);
    
    display.clearDisplay();
    display.display();
    delay(100);
    
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    display.clearDisplay();
    display.setCursor(20, 20);
    display.println(F("Display OK!"));
    display.setCursor(15, 35);
    display.print(F("Addr: 0x"));
    display.println(displayAddress, HEX);
    display.display();
    delay(1000);
  }
  
  pinMode(BUTTON_JUMP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  
  resetGame();
  
  if(displayFound) {
    display.clearDisplay();
    display.setCursor(20, 20);
    display.println(F("Geometry Dash"));
    display.setCursor(25, 35);
    display.println(F("Press to"));
    display.setCursor(30, 45);
    display.println(F("start!"));
    display.display();
    
    while(digitalRead(BUTTON_JUMP) == HIGH) {
      delay(10);
    }
    delay(200);
  } else {
    Serial.println(F("\nPress button to continue (display not available)"));
    while(digitalRead(BUTTON_JUMP) == HIGH) {
      delay(10);
    }
    delay(200);
  }
}

void loop() {
  if (!displayFound) {
    Serial.println(F("Display not available - check connections"));
    delay(5000);
    return;
  }
  
  if (gameOver) {
    display.clearDisplay();
    display.setCursor(35, 20);
    display.println(F("GAME OVER"));
    display.setCursor(20, 35);
    display.print(F("Score: "));
    display.println(score);
    display.setCursor(15, 50);
    display.println(F("Press to retry"));
    display.display();
    
    if (digitalRead(BUTTON_JUMP) == LOW) {
      delay(200);
      resetGame();
    }
    return;
  }
  
  unsigned long currentTime = millis();
  if (currentTime - lastFrameTime < 16) {
    return;
  }
  lastFrameTime = currentTime;
  
  if (digitalRead(BUTTON_JUMP) == LOW && (!player.isJumping || player.onPlatform)) {
    player.velocityY = -4.5;
    player.isJumping = true;
    player.onPlatform = false;
  }
  
  if (digitalRead(BUTTON_DOWN) == LOW && player.isJumping) {
    player.velocityY = 8.0;
    player.onPlatform = false;
  }
  
  bool wasOnPlatform = player.onPlatform;
  player.onPlatform = false;
  bool hitTrampoline = false;
  int trampolineIndex = -1;
  
  for (int i = 0; i < 5; i++) {
    if (obstacles[i].active) {
      if (obstacles[i].type == OBSTACLE_PLATFORM) {
        if (player.x + player.width > obstacles[i].x && 
            player.x < obstacles[i].x + obstacles[i].width &&
            player.y + player.height >= obstacles[i].gapY - obstacles[i].height &&
            player.y + player.height <= obstacles[i].gapY + 2 &&
            player.velocityY >= 0) {
          player.y = obstacles[i].gapY - player.height;
          player.velocityY = 0;
          player.isJumping = false;
          player.onPlatform = true;
          player.platformY = obstacles[i].gapY;
        }
      } else if (obstacles[i].type == OBSTACLE_TRAMPOLINE) {
        if (player.x + player.width > obstacles[i].x && 
            player.x < obstacles[i].x + obstacles[i].width &&
            player.y + player.height >= player.groundY - obstacles[i].height &&
            player.y + player.height <= player.groundY + 2 &&
            player.velocityY >= 0) {
          hitTrampoline = true;
          trampolineIndex = i;
        }
      }
    }
  }
  
  if (hitTrampoline) {
    player.velocityY = -7.5;
    player.isJumping = true;
    player.y = player.groundY - obstacles[trampolineIndex].height - player.height - 1;
  }
  
  player.y += player.velocityY;
  player.velocityY += 0.3;
  
  if (player.y >= player.groundY - player.height) {
    player.y = player.groundY - player.height;
    player.velocityY = 0;
    player.isJumping = false;
    player.onPlatform = false;
  }
  
  if (player.onPlatform && digitalRead(BUTTON_DOWN) == LOW) {
    player.onPlatform = false;
    player.isJumping = true;
    player.velocityY = 1.0;
  }
  
  if (player.onPlatform && !player.isJumping) {
    bool platformExists = false;
    for (int i = 0; i < 5; i++) {
      if (obstacles[i].active && obstacles[i].type == OBSTACLE_PLATFORM) {
        if (player.x + player.width > obstacles[i].x && 
            player.x < obstacles[i].x + obstacles[i].width) {
          platformExists = true;
          player.y = obstacles[i].gapY - player.height;
          break;
        }
      }
    }
    if (!platformExists) {
      player.onPlatform = false;
      player.isJumping = true;
    }
  }
  
  if (currentTime - lastObstacleTime > obstacleSpawnInterval) {
    spawnObstacle();
    lastObstacleTime = currentTime;
  }
  
  for (int i = 0; i < 5; i++) {
    if (obstacles[i].active) {
      obstacles[i].x -= gameSpeed;
      
      if (obstacles[i].x + obstacles[i].width < 0) {
        obstacles[i].active = false;
        score += 10;
      }
      
      if (checkCollision(player, obstacles[i])) {
        gameOver = true;
      }
    }
  }
  
  if (score > 0 && score % 200 == 0) {
    gameSpeed = 2 + (score / 200);
    if (gameSpeed > 5) gameSpeed = 5;
  }
  
  drawGame();
}

void scanI2C() {
  byte error, address;
  int nDevices = 0;

  Serial.println(F("Scanning I2C addresses (1-126)..."));
  Serial.println(F("Address\t\tStatus"));
  Serial.println(F("-------\t\t------"));

  for(address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print(F("0x"));
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.print(F("\t\t✓ Found!"));
      if(address == 0x3C || address == 0x3D) {
        Serial.print(F(" (SSD1306?)"));
      }
      Serial.println();
      nDevices++;
    } else if (error == 4) {
      Serial.print(F("0x"));
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println(F("\t\t? Unknown error"));
    }
  }
  
  Serial.println();
  if (nDevices == 0) {
    Serial.println(F("❌ NO I2C devices found!"));
    Serial.println(F("\nPossible causes:"));
    Serial.println(F("1. Wrong SDA/SCL connections"));
    Serial.print(F("   Check: SDA on GPIO "));
    Serial.print(SDA_PIN);
    Serial.print(F(", SCL on GPIO "));
    Serial.println(SCL_PIN);
    Serial.println(F("2. Display not powered correctly"));
    Serial.println(F("3. Defective display"));
    Serial.println(F("4. Different I2C pins (try modifying SDA_PIN/SCL_PIN)"));
  } else {
    Serial.print(F("✓ Found "));
    Serial.print(nDevices);
    Serial.println(F(" I2C device(s)"));
    if(nDevices > 0) {
      Serial.println(F("\nIf you see 0x3C or 0x3D, the display is connected!"));
    }
  }
}

void resetGame() {
  player.x = 10;
  player.y = player.groundY - player.height;
  player.velocityY = 0;
  player.isJumping = false;
  player.onPlatform = false;
  player.platformY = 0;
  
  for (int i = 0; i < 5; i++) {
    obstacles[i].active = false;
    obstacles[i].type = OBSTACLE_LOW;
    obstacles[i].gapY = 0;
    obstacles[i].gapHeight = 0;
    obstacles[i].gapX = 0;
  }
  
  score = 0;
  gameSpeed = 2;
  gameOver = false;
  lastObstacleTime = millis();
  lastFrameTime = millis();
  randomSeed(analogRead(0) + millis());
  patternSeed = random(0, 1000);
}

void spawnObstacle() {
  int patternType = random(0, 11);
  
  if (patternType >= 8) {
    patternType += 1;
  }
  
  if (patternType == 8) {
    for (int i = 0; i < 5; i++) {
      if (!obstacles[i].active) {
        obstacles[i].type = OBSTACLE_TRAMPOLINE;
        obstacles[i].height = 4;
        obstacles[i].width = 10;
        obstacles[i].gapY = 0;
        obstacles[i].gapHeight = 0;
        obstacles[i].gapX = 0;
        obstacles[i].x = SCREEN_WIDTH - 50;
        obstacles[i].active = true;
        break;
      }
    }
  }
  
  for (int i = 0; i < 5; i++) {
    if (!obstacles[i].active) {
      if (patternType == 8) {
        obstacles[i].type = OBSTACLE_ULTRA_HIGH;
        obstacles[i].height = 40;
        obstacles[i].width = 6;
        obstacles[i].gapY = 0;
        obstacles[i].gapHeight = 0;
        obstacles[i].gapX = 0;
      } else {
        generateObstaclePattern(i, patternType);
      }
      obstacles[i].x = SCREEN_WIDTH;
      obstacles[i].active = true;
      break;
    }
  }
}

void generateObstaclePattern(int index, int patternType) {
  if (patternType == -1) {
    patternType = random(0, 12);
  }
  
  switch(patternType) {
    case 0:
      obstacles[index].type = OBSTACLE_LOW;
      obstacles[index].height = 12;
      obstacles[index].width = 6;
      break;
    case 1:
      obstacles[index].type = OBSTACLE_HIGH;
      obstacles[index].height = 25;
      obstacles[index].width = 6;
      break;
    case 2:
      obstacles[index].type = OBSTACLE_MEDIUM;
      obstacles[index].height = 18;
      obstacles[index].width = 8;
      break;
    case 3:
      obstacles[index].type = OBSTACLE_VERY_HIGH;
      obstacles[index].height = 30;
      obstacles[index].width = 6;
      break;
    case 4:
      obstacles[index].type = OBSTACLE_FLYING;
      obstacles[index].height = 10;
      obstacles[index].width = 8;
      obstacles[index].gapY = player.groundY - 35;
      break;
    case 5:
      obstacles[index].type = OBSTACLE_GAP;
      obstacles[index].height = 10;
      obstacles[index].width = 5;
      obstacles[index].gapY = player.groundY - 22;
      obstacles[index].gapHeight = 22;
      obstacles[index].gapX = 8;
      break;
    case 6:
      obstacles[index].type = OBSTACLE_WIDE;
      obstacles[index].height = 20;
      obstacles[index].width = 15;
      break;
    case 7:
      obstacles[index].type = OBSTACLE_NARROW;
      obstacles[index].height = 28;
      obstacles[index].width = 4;
      break;
    case 8:
      obstacles[index].type = OBSTACLE_ULTRA_HIGH;
      obstacles[index].height = 40;
      obstacles[index].width = 6;
      break;
    case 9:
      obstacles[index].type = OBSTACLE_PLATFORM;
      obstacles[index].height = 3;
      obstacles[index].width = 12;
      obstacles[index].gapY = player.groundY - 30;
      break;
    case 10:
      obstacles[index].type = OBSTACLE_TRAMPOLINE;
      obstacles[index].height = 4;
      obstacles[index].width = 10;
      break;
    case 11:
      obstacles[index].type = OBSTACLE_DUCK;
      obstacles[index].height = 12;
      obstacles[index].width = 8;
      obstacles[index].gapY = player.groundY - 20;
      obstacles[index].gapHeight = 8;
      break;
    default:
      obstacles[index].type = OBSTACLE_MEDIUM;
      obstacles[index].height = 15;
      obstacles[index].width = 6;
      break;
  }
}

bool checkCollision(Player p, Obstacle o) {
  if (p.x + p.width > o.x && p.x < o.x + o.width) {
    switch(o.type) {
      case OBSTACLE_FLYING:
        if (p.y < o.gapY + o.height && p.y + p.height > o.gapY) {
          return true;
        }
        break;
      case OBSTACLE_GAP: {
        float gapTop = o.gapY - o.gapHeight / 2;
        float gapBottom = o.gapY + o.gapHeight / 2;
        
        bool inGapZone = (p.y + p.height > gapTop && p.y < gapBottom);
        
        if (!inGapZone) {
          if (p.x + p.width > o.x && p.x < o.x + o.width) {
            if (p.y + p.height > gapTop - o.height && p.y < gapTop) {
              return true;
            }
          }
          float bottomBlockX = o.x - o.gapX;
          if (p.x + p.width > bottomBlockX && p.x < bottomBlockX + o.width) {
            if (p.y + p.height > player.groundY - o.height) {
              return true;
            }
          }
        }
        break;
      }
      case OBSTACLE_PLATFORM:
        break;
      case OBSTACLE_TRAMPOLINE:
        break;
      case OBSTACLE_ULTRA_HIGH:
        if (p.y + p.height > player.groundY - o.height) {
          return true;
        }
        break;
      case OBSTACLE_DUCK: {
        if (p.y + p.height > player.groundY - o.height) {
          return true;
        }
        float ceilingY = o.gapY - o.gapHeight / 2;
        if (p.y < ceilingY + o.height && p.y + p.height > ceilingY) {
          return true;
        }
        break;
      }
      default:
        if (p.y + p.height > player.groundY - o.height) {
          return true;
        }
        break;
    }
  }
  return false;
}

void drawGame() {
  display.clearDisplay();
  
  display.drawLine(0, player.groundY, SCREEN_WIDTH, player.groundY, SSD1306_WHITE);
  
  display.fillRect(player.x, player.y, player.width, player.height, SSD1306_WHITE);
  
  for (int i = 0; i < 5; i++) {
    if (obstacles[i].active) {
      switch(obstacles[i].type) {
        case OBSTACLE_FLYING:
          display.fillRect(obstacles[i].x, obstacles[i].gapY, obstacles[i].width, obstacles[i].height, SSD1306_WHITE);
          break;
        case OBSTACLE_GAP: {
          int gapTop = obstacles[i].gapY - obstacles[i].gapHeight / 2;
          int gapBottom = obstacles[i].gapY + obstacles[i].gapHeight / 2;
          
          display.fillRect(obstacles[i].x, gapTop - obstacles[i].height, 
                          obstacles[i].width, obstacles[i].height, SSD1306_WHITE);
          int bottomBlockX = obstacles[i].x - obstacles[i].gapX;
          display.fillRect(bottomBlockX, player.groundY - obstacles[i].height, 
                          obstacles[i].width, obstacles[i].height, SSD1306_WHITE);
          break;
        }
        case OBSTACLE_PLATFORM: {
          display.fillRect(obstacles[i].x, obstacles[i].gapY - obstacles[i].height, 
                          obstacles[i].width, obstacles[i].height, SSD1306_WHITE);
          break;
        }
        case OBSTACLE_TRAMPOLINE: {
          display.fillRect(obstacles[i].x, player.groundY - obstacles[i].height, 
                          obstacles[i].width, obstacles[i].height, SSD1306_WHITE);
          display.drawLine(obstacles[i].x + 2, player.groundY - obstacles[i].height - 2, 
                          obstacles[i].x + 2, player.groundY - obstacles[i].height, SSD1306_WHITE);
          display.drawLine(obstacles[i].x + obstacles[i].width - 3, player.groundY - obstacles[i].height - 2, 
                          obstacles[i].x + obstacles[i].width - 3, player.groundY - obstacles[i].height, SSD1306_WHITE);
          break;
        }
        case OBSTACLE_ULTRA_HIGH: {
          int obstacleY = player.groundY - obstacles[i].height;
          display.fillRect(obstacles[i].x, obstacleY, obstacles[i].width, obstacles[i].height, SSD1306_WHITE);
          break;
        }
        case OBSTACLE_DUCK: {
          int ceilingY = obstacles[i].gapY - obstacles[i].gapHeight / 2;
          display.fillRect(obstacles[i].x, player.groundY - obstacles[i].height, 
                          obstacles[i].width, obstacles[i].height, SSD1306_WHITE);
          display.fillRect(obstacles[i].x, ceilingY - obstacles[i].height, 
                          obstacles[i].width, obstacles[i].height, SSD1306_WHITE);
          break;
        }
        default:
          int obstacleY = player.groundY - obstacles[i].height;
          display.fillRect(obstacles[i].x, obstacleY, obstacles[i].width, obstacles[i].height, SSD1306_WHITE);
          break;
      }
    }
  }
  
  display.setCursor(5, 5);
  display.print(F("Score: "));
  display.print(score);
  
  display.setCursor(5, 15);
  display.print(F("Speed: "));
  display.print(gameSpeed);
  
  display.display();
}
