#include <Wire.h>
#include "SPI.h"
#include <Adafruit_GFX.h>
#include "ILI9488.h"

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

#define NS2009_ADDR 0x48 //10010000

#define NS2009_LOW_POWER_READ_X 0xc0
#define NS2009_LOW_POWER_READ_Y 0xd0
#define NS2009_LOW_POWER_READ_Z1 0xe0

ILI9488 tft = ILI9488(TFT_CS, TFT_DC, TFT_RST);

int vx = 0;
int vy = 0;

int ns2009_recv(const uint8_t *send_buf, size_t send_buf_len, uint8_t *receive_buf,
                size_t receive_buf_len)
{
    Wire.beginTransmission(NS2009_ADDR);
    Wire.write(send_buf, send_buf_len);
    Wire.endTransmission();
    Wire.requestFrom(NS2009_ADDR, receive_buf_len);
    while (Wire.available())
    {
        *receive_buf++ = Wire.read();
    }
    return 0;
}

uint8_t ns2009_read(uint8_t cmd, int *val)
{
    uint8_t ret, buf[2];
    ret = ns2009_recv(&cmd, 1, buf, 2);
    if (ret != 0)
        return 0;
    if (val)
        *val = (buf[0] << 4) | (buf[1] >> 4);
    return 1;
}

void setup()
{

    Serial.begin(115200);
    while (!Serial)
        ; // Leonardo: wait for serial monitor
    Serial.println("\n NS2009 test");

    Wire.begin(ESP32_SDA, ESP32_SCL);
    byte error, address;

    Wire.beginTransmission(NS2009_ADDR);
    error = Wire.endTransmission();

    if (error == 0)
    {
        Serial.print("I2C device found at address 0x");
        Serial.print(NS2009_ADDR, HEX);
        Serial.println("  !");
    }
    else if (error == 4)
    {
        Serial.print("Unknown error at address 0x");
        Serial.println(NS2009_ADDR, HEX);
    }
    int i = 0;

    int temp = 0;
    for (i = 0; i < 3; i++)
    {
        ns2009_read(NS2009_LOW_POWER_READ_X, &temp);
        vx += temp;
    }
    vx = vx / 3;
    vx = 4095;
    temp = 0;
    for (i = 0; i < 3; i++)
    {
        ns2009_read(NS2009_LOW_POWER_READ_X, &temp);
        vy += temp;
    }
    vy = vy / 3;
    vy = 4095;

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
    int x = 0;
    int y = 0;
    int z1 = 0;
    int x_addr = 0;
    int y_addr = 0;

    ns2009_read(NS2009_LOW_POWER_READ_X, &x);
    ns2009_read(NS2009_LOW_POWER_READ_Y, &y);
    ns2009_read(NS2009_LOW_POWER_READ_Z1, &z1);
    x_addr = x * 320 / vx;
    y_addr = y * 480 / vy;
    Serial.println(x);
    Serial.println(y);
    Serial.println(z1);
    Serial.println(x_addr);
    Serial.println(y_addr);
    Serial.println("...........");

    tft.drawPixel(x_addr,y_addr,ILI9488_RED);
    tft.fillRect(x_addr, y_addr, 5, 5, ILI9488_RED);

    //delay(100); // wait 5 seconds for next scan
}