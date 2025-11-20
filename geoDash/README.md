# Geometry Dash - ESP32

Un clone di Geometry Dash implementato per Adafruit Feather ESP32 V2 con display OLED SSD1306.

## Hardware Richiesto

- **Board**: Adafruit Feather ESP32 V2
- **Display**: OLED SSD1306 (128x64)
- **Pulsanti**: 
  - Jump button su GPIO 14
  - Down button su GPIO 12

## Connessioni

### Display OLED
- **SDA** → GPIO 22
- **SCL** → GPIO 20
- **VCC** → 3.3V
- **GND** → GND

### Pulsanti
- **Jump Button** → GPIO 14 (con pull-up interno)
- **Down Button** → GPIO 12 (con pull-up interno)

## Caratteristiche

- Gioco a scorrimento infinito
- Vari tipi di ostacoli (bassi, alti, volanti, piattaforme, trampolini, ecc.)
- Sistema di punteggio
- Velocità di gioco progressiva
- Fisica di salto e caduta
- Supporto per display OLED 128x64

## Come Compilare e Caricare

1. Installa PlatformIO
2. Apri il progetto in PlatformIO
3. Collega la board ESP32
4. Clicca su "Upload" per caricare il codice

## Controlli

- **Pulsante Jump**: Salta
- **Pulsante Down**: Scendi velocemente (quando in aria) o scendi da una piattaforma

## Librerie Utilizzate

- Adafruit SSD1306
- Adafruit GFX Library
- Adafruit BusIO

## Note

Il gioco include diagnostica I2C per verificare la connessione del display. Controlla il monitor seriale (115200 baud) per informazioni di debug.

