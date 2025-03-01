#include "HS12864TG10B.h"

#define PIN_SDA 12  // MOSI
#define PIN_SCK 13  // SCK
#define PIN_CS 18   // Chip Select
#define PIN_A0 17   // Data/Command (RS en el código original)
#define PIN_RST 16  // Reset (elige un pin disponible)

HS12864TG10B display(PIN_SDA, PIN_SCK, PIN_CS, PIN_A0, PIN_RST);

void setup() {
  // put your setup code here, to run once:
  display.init();
  display.flip(false);

  display.setTextColor(1);
  display.setTextSize(1);
  display.setCursor(0,0);

  display.println("Hello World!");

  display.setCursor(0,63-8);

  display.println("Lucho Tecnologies");

  display.display();
}

void loop() {
  
}
