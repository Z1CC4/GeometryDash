# GeoDash

A Geometry Dash–inspired jump-and-run game for **ESP32** with an **SSD1306 OLED** display.

University project (Free University of Bozen-Bolzano) by [Andrea Zicarelli](https://github.com/Z1CC4) and [Elias Vieider](https://github.com/vieiderElias).

## Features

- Three playable shapes: square, circle, triangle
- Physics-based jumping with gravity
- Dynamic obstacles and progressive difficulty
- Score and high-score
- Game states: menu → playing → game over
- Runs on a physical ESP32 or in the [Wokwi](https://wokwi.com/) simulator

## Hardware

| Component | Type |
|-----------|------|
| Microcontroller | ESP32 DevKit (or compatible) |
| Display | SSD1306 OLED 128×64, I2C |
| Input | 3× push buttons (jump, left, right) |

Default wiring (Wokwi / typical ESP32 DevKit):

- OLED SDA → GPIO 21, SCL → GPIO 22
- Jump → GPIO 14, Left → GPIO 27, Right → GPIO 26

If you use a different board (e.g. Adafruit Feather), change the I2C pins in `setup()` with `Wire.begin(SDA_PIN, SCL_PIN)`.

## Project layout

```
geoDashForSimulation/   Wokwi simulator build (PlatformIO)
geoDashForESP32/        firmware for a physical ESP32
geoDash/                extra PlatformIO sketch
GeoDashPPT.pptx         project presentation
```

## Build

Requires [VS Code](https://code.visualstudio.com/) + [PlatformIO](https://platformio.org/).

```bash
git clone https://github.com/Z1CC4/GeometryDash.git
cd GeometryDash/geoDashForSimulation
pio run
```

Libraries (installed by PlatformIO): Adafruit SSD1306, Adafruit GFX, Adafruit BusIO.

- **Simulator:** compile, then `Wokwi: Start Simulator` in VS Code.
- **Hardware:** `cd geoDashForESP32 && pio run --target upload`

## Controls

- Left / Right: change shape (menu) or navigate game-over options
- Jump / Start: jump in-game, confirm in menus

## Authors

- Andrea Zicarelli ([@Z1CC4](https://github.com/Z1CC4))
- Elias Vieider ([@vieiderElias](https://github.com/vieiderElias))
