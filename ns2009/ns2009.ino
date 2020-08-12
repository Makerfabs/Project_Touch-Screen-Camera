#include <Wire.h>

#define ESP32_SDA 26
#define ESP32_SCL 27

#define NS2009_ADDR 0x48 //10010000

void setup()
{

    Serial.begin(115200);
    while (!Serial)
        ; // Leonardo: wait for serial monitor
    Serial.println("\n NS2009 test");
}

void loop()
{
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

    delay(2000); // wait 5 seconds for next scan
}