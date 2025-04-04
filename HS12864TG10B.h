//HS HS12864TG10B driver library with Adafruit_GFX. Written by Lucho Tecnologies with the help of chatGpt :P
//Last edit 19/03/25 - 20:15

//COM SPEED. Min delay recomended in datasheet is 50ns
#ifndef HW_HS12864TG10B
#define HW_HS12864TG10B

#define SPI_DELAY_NS 55

//COLORS:

#define HS12864_BLACK 1
#define HS12864_WHITE 0
#define HS12864_INVERSE 2



#include <Arduino.h>
#include <Adafruit_GFX.h>

uint8_t screenBuffer[128 * 8] = { 0 };  // Inicializa el buffer en 0
uint8_t* screenBuffer2;
bool inTransition;
//HS12864TG10B display;  class declaration example

void delayNS(uint32_t ns) {
  delayMicroseconds(ns / 1000);  // Convertimos nanosegundos a microsegundos
}

class HS12864TG10B : public Adafruit_GFX {
private:
  uint8_t pin_sda, pin_sck, pin_cs, pin_a0, pin_rst, pin_light;
  bool screenflip;
public : using Adafruit_GFX::Adafruit_GFX;  // Heredar constructor
  void clearDisplay() {
	  if(!inTransition){
		memset(screenBuffer, 0, sizeof(screenBuffer));
	  }else{
		memset(screenBuffer2, 0, sizeof(screenBuffer));
	  }
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
	if(!inTransition){
		if (color == 1)
		  screenBuffer[index] |= bitMask;  // Encender píxel
		else if (color == 0) {
		  screenBuffer[index] &= ~bitMask;  // Apagar píxel
		} else {
		  screenBuffer[index] ^= bitMask;  // Invertir píxel
		}
	}else{
		if (color == 1)
		  screenBuffer2[index] |= bitMask;  // Encender píxel
		else if (color == 0) {
		  screenBuffer2[index] &= ~bitMask;  // Apagar píxel
		} else {
		  screenBuffer2[index] ^= bitMask;  // Invertir píxel
		}
	}
  }
  void updateDisplay();
  void flip(bool value);
  void setContrast(uint8_t value);
  void setBrightness(uint8_t brightness);
  void lightPin(int pin);
  uint8_t* getBuffer() {
    return screenBuffer;
  }
  void begin(){
	init();
  }
  void writeRawBuffer(const uint8_t* data, size_t length) {
    if (length > sizeof(screenBuffer)) {
        length = sizeof(screenBuffer); // Evitar overflow
    }
    memcpy(screenBuffer, data, length);
  }
  
  void startTransitionWrite(){
	  inTransition = true;
	  screenBuffer2 = (uint8_t*)malloc(128 * 8);
	  memset(screenBuffer2, 0, 128 * 8);
  }
  
  uint8_t drawTransition(int step){
	//draw first buffer
	uint8_t total = 0;
	
	for (uint8_t page = 0; page < 8; page++) {
		int npage = page - step;
		if(npage >= 0){
			writeCommand(0xB0 | page);  // Seleccionar página
			writeCommand(0x10);         // Dirección de columna alta
			writeCommand(0x00);         // Dirección de columna baja
			for (uint8_t col = 0; col < 128; col++) {
				writeData(screenBuffer[npage * 128 + col]);
			}
			total++;
		}else{
			/*
			writeCommand(0xB0 | page);  // Seleccionar página
			writeCommand(0x10);         // Dirección de columna alta
			writeCommand(0x00);         // Dirección de columna baja
			for (uint8_t col = 0; col < 128; col++) {
				writeData(0x00);
			}*/
		}
	}
	
	for (uint8_t page = 0; page < 8; page++) {
		int npage = page + 8 - step;
		if(npage < 8){
			writeCommand(0xB0 | page);  // Seleccionar página
			writeCommand(0x10);         // Dirección de columna alta
			writeCommand(0x00);         // Dirección de columna baja
			for (uint8_t col = 0; col < 128; col++) {
				writeData(screenBuffer2[npage * 128 + col]);
			}
			total++;
		}
	}
	/*
	
	//draw second buffer
	for (uint8_t page = 0; page < 8; page++) {
		uint8_t npage = page + step;
		if(npage < 8){
			writeCommand(0xB0 | npage);  // Seleccionar página
			writeCommand(0x10);         // Dirección de columna alta
			writeCommand(0x00);         // Dirección de columna baja
			for (uint8_t col = 0; col < 128; col++) {
				writeData(screenBuffer2[npage * 128 + col]);
			}
			total++;
		}
		
	}
	*/
	
	return total;
  }
  
  void endTransition(){
	  inTransition = false;
	  free(screenBuffer2);
  }
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

void HS12864TG10B::lightPin(int pin){
	pinMode(pin, OUTPUT);
	pin_light = pin;
}

void HS12864TG10B::setContrast(uint8_t value) {
  uint8_t contrast = map(value, 0, 255, 0 , 63);

  writeCommand(0x81);  // Set contrast
  writeCommand(contrast);  // VOP (ajustar según pruebas)
}
#ifdef ESP32
void HS12864TG10B::setBrightness(uint8_t brightness) {
  if (brightness == 0) {
    ledcDetachPin(pin_light);
    digitalWrite(pin_light, LOW);
  } else {
    ledcAttachPin(pin_light, 0);
    ledcSetup(0, 5000, 8);
    ledcWrite(0, brightness);
  }
}
#else
void HS12864TG10B::setBrightness(uint8_t brightness) {
	Serial.println("Only avaiable for ESP32");
}
#endif

#endif
