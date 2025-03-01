//HS HS12864TG10B driver library with Adafruit_GFX. Written by Lucho Tecnologies with the help of chatGpt :P
//Last edit 1/03/25 - 16:35

//COM SPEED. Min delay recomended in datasheet is 50ns

#define SPI_DELAY_NS 55

//COLORS:

#define HS12864_BLACK 0
#define HS12864_WHITE 1
#define HS12864_INVERSE 2



#include <Arduino.h>
#include <Adafruit_GFX.h>

uint8_t screenBuffer[128 * 8] = { 0 };  // Inicializa el buffer en 0

//HS12864TG10B display;  class declaration example

void delayNS(uint32_t ns) {
  delayMicroseconds(ns / 1000);  // Convertimos nanosegundos a microsegundos
}

class HS12864TG10B : public Adafruit_GFX {
private:
  uint8_t pin_sda, pin_sck, pin_cs, pin_a0, pin_rst;
  bool screenflip;
public : using Adafruit_GFX::Adafruit_GFX;  // Heredar constructor
  void clearDisplay() {
    memset(screenBuffer, 0, sizeof(screenBuffer));
  }
  void display() {
    updateDisplay();
  }
public:
  HS12864TG10B(uint8_t sda, uint8_t sck, uint8_t cs, uint8_t a0, uint8_t rst)
    : Adafruit_GFX(128, 64), pin_sda(sda), pin_sck(sck), pin_cs(cs), pin_a0(a0), pin_rst(rst) {}
  void init();
  void writeCommand(uint8_t cmd);
  void writeData(uint8_t data);
  void drawPixel(int16_t x, int16_t y, uint16_t color) override {
    if (screenflip) {
      x = 127 - x;  // Invertir en eje X
      y = 63 - y;   // Invertir en eje Y
    }

    if (x >= 128 || y >= 64 || x < 0 || y < 0) return;  // Fuera de rango
    uint16_t index = (y / 8) * 128 + x;
    uint8_t bitMask = 1 << (y % 8);
    if (color == 1)
      screenBuffer[index] |= bitMask;  // Encender píxel
    else if (color == 0) {
      screenBuffer[index] &= ~bitMask;  // Apagar píxel
    } else {
      screenBuffer[index] ^= bitMask;  // Invertir píxel
    }
  }
  void updateDisplay();
  void flip(bool value);
  void setContrast(uint8_t value);
};

void HS12864TG10B::writeCommand(uint8_t cmd) {
  digitalWrite(pin_cs, LOW);
  digitalWrite(pin_a0, LOW);  // Modo comando
  for (uint8_t i = 0; i < 8; i++) {
    digitalWrite(pin_sda, (cmd & 0x80) ? HIGH : LOW);
    cmd <<= 1;
    digitalWrite(pin_sck, HIGH);
    delayNS(SPI_DELAY_NS);
    digitalWrite(pin_sck, LOW);
    delayNS(SPI_DELAY_NS);
  }
  digitalWrite(pin_cs, HIGH);
}

void HS12864TG10B::writeData(uint8_t data) {
  digitalWrite(pin_cs, LOW);
  digitalWrite(pin_a0, HIGH);  // Modo datos
  for (uint8_t i = 0; i < 8; i++) {
    digitalWrite(pin_sda, (data & 0x80) ? HIGH : LOW);
    data <<= 1;
    digitalWrite(pin_sck, HIGH);
    delayNS(SPI_DELAY_NS);
    digitalWrite(pin_sck, LOW);
    delayNS(SPI_DELAY_NS);
  }
  digitalWrite(pin_cs, HIGH);
}

void HS12864TG10B::init() {
  pinMode(pin_cs, OUTPUT);
  pinMode(pin_a0, OUTPUT);
  pinMode(pin_rst, OUTPUT);
  pinMode(pin_sck, OUTPUT);
  pinMode(pin_sda, OUTPUT);

  digitalWrite(pin_rst, HIGH);
  delay(10);
  digitalWrite(pin_rst, LOW);
  delay(10);
  digitalWrite(pin_rst, HIGH);
  delay(10);

  writeCommand(0xe3);  // Reset interno
  writeCommand(0xa3);  // Bias 1/9
  writeCommand(0xA0);  // Restaurar dirección de columnas // ADC normal
  writeCommand(0xC8);  // Restaurar dirección de filas // Com Scan reverse
  writeCommand(0x2f);  // Power control
  writeCommand(0x24);  // Resistor ratio
  writeCommand(0x81);  // Set contrast
  writeCommand(0x16);  // VOP (ajustar según pruebas)
  writeCommand(0xf8);  // Booster ratio
  writeCommand(0x08);
  writeCommand(0xA6);  // NOT INVERT SCREEN
  writeCommand(0xaf);  // Display ON
}


void HS12864TG10B::updateDisplay() {
  for (uint8_t page = 0; page < 8; page++) {
    writeCommand(0xB0 | page);  // Seleccionar página
    writeCommand(0x10);         // Dirección de columna alta
    writeCommand(0x00);         // Dirección de columna baja
    for (uint8_t col = 0; col < 128; col++) {
      writeData(screenBuffer[page * 128 + col]);
    }
  }
}

void HS12864TG10B::flip(bool value) {
  screenflip = value;
}


void HS12864TG10B::setContrast(uint8_t value) {
  uint8_t contrast = map(value, 0, 255, 0 , 63);

  writeCommand(0x81);  // Set contrast
  writeCommand(contrast);  // VOP (ajustar según pruebas)
}
