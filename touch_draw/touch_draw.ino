#include <Wire.h>
#include "SPI.h"
#include <Adafruit_GFX.h>
#include "ILI9488.h"
#include "NS2009.h"

//SPI
#define SPI_MOSI 13
#define SPI_MISO 12
#define SPI_SCK 14

//TFT
#define TFT_CS 15
#define TFT_DC 33
#define TFT_LED -1 //1//-1
#define TFT_RST -1 //3//-1

//SPI control
#define SPI_ON_TFT digitalWrite(TFT_CS, LOW)
#define SPI_OFF_TFT digitalWrite(TFT_CS, HIGH)

#define ESP32_SDA 26
#define ESP32_SCL 27

//ft6236
#define TOUCH_I2C_ADD 0x38

#define TOUCH_REG_XL 0x04
#define TOUCH_REG_XH 0x03
#define TOUCH_REG_YL 0x06
#define TOUCH_REG_YH 0x05

#define NS2009_TOUCH
//#define FT6236_TOUCH

#ifdef NS2009_TOUCH
const int i2c_touch_addr = NS2009_ADDR;
#endif
#ifdef FT6236_TOUCH
const int i2c_touch_addr = TOUCH_I2C_ADD;
#endif

ILI9488 tft = ILI9488(TFT_CS, TFT_DC, TFT_RST);

int last_pos[2] = {0, 0};

void setup()
{

    Serial.begin(115200);
    while (!Serial)
        ; // Leonardo: wait for serial monitor
    Serial.println("\n NS2009 test");

    Wire.begin(ESP32_SDA, ESP32_SCL);
    byte error, address;

    Wire.beginTransmission(i2c_touch_addr);
    error = Wire.endTransmission();

    if (error == 0)
    {
        Serial.print("I2C device found at address 0x");
        Serial.print(i2c_touch_addr, HEX);
        Serial.println("  !");
    }
    else if (error == 4)
    {
        Serial.print("Unknown error at address 0x");
        Serial.println(i2c_touch_addr, HEX);
    }

    pinMode(TFT_CS, OUTPUT);
    SPI_OFF_TFT;
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

    //TFT(SPI) init
    SPI_ON_TFT;
    tft.begin();
    tft.fillScreen(ILI9488_BLUE);
    SPI_OFF_TFT;
}

void loop()
{

    int pos[2] = {0, 0};
    ns2009_pos(pos);
    if (filter(last_pos, pos, 18))
    {
        tft.fillRect(pos[0], pos[1], 3, 3, ILI9488_RED);
    }
}

int filter(int last_pos[2], int pos[2], int level)
{
    int temp = (last_pos[0] - pos[0]) * (last_pos[0] - pos[0]) + (last_pos[1] - pos[1]) * (last_pos[1] - pos[1]);
    last_pos[0] = pos[0];
    last_pos[1] = pos[1];
    if (temp > level)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}