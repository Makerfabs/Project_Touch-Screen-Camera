/***************************************************
  This is our GFX example for the Adafruit ILI9488 Breakout and Shield
  ----> http://www.adafruit.com/products/1651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

#include "SPI.h"
#include <Adafruit_GFX.h>
#include "ILI9488.h"

//SPI
#define SPI_MOSI 13
#define SPI_MISO 12
#define SPI_SCK 14

//SD Card
#define SD_CS 4

//TFT
#define TFT_CS 15
#define TFT_DC 33
#define TFT_LED -1 //1//-1
#define TFT_RST -1 //3//-1

//SPI control
#define SPI_ON_TFT  digitalWrite(TFT_CS, LOW)
#define SPI_OFF_TFT  digitalWrite(TFT_CS, HIGH)

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
ILI9488 tft = ILI9488(TFT_CS, TFT_DC, TFT_RST);
// If using the breakout, change pins as desired
//ILI9488 tft = ILI9488(TFT_CS, TFT_DC, SPI_MOSI, SPI_SCK, TFT_RST, SPI_MISO);


void setup()
{
  Serial.begin(115200);

  //SPI init
  pinMode(TFT_CS, OUTPUT);
  SPI_OFF_TFT;
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI); 

  //TFT(SPI) init
  SPI_ON_TFT;
  tft.begin();
  SPI_OFF_TFT;

  SPI.setFrequency(24000000);
}

void loop(void)
{
  SPI_ON_TFT;
  screen_test();
  SPI_OFF_TFT;
}

void screen_test()
{
  Serial.print(F("Screen fill              "));
  Serial.println(testFillScreen());
}

unsigned long testFillScreen()
{
  unsigned long start = micros();
  tft.fillScreen(ILI9488_BLACK);
  tft.fillScreen(ILI9488_RED);
  tft.fillScreen(ILI9488_GREEN);
  tft.fillScreen(ILI9488_BLUE);
  tft.fillScreen(ILI9488_BLACK);
  return micros() - start;
}