#include "SPI.h"
#include "TFT_eSPI.h"
#include <Wire.h>
#include "Adafruit_SGP30.h"
#include "DHT.h"

#define BUZZER_ON digitalWrite(BUZZPIN, HIGH)
#define BUZZER_OFF digitalWrite(BUZZPIN, LOW)
#define LDO_1V8_ON digitalWrite(LDO_PWR_EN_PIN, HIGH)
#define LDO_1V8_OFF digitalWrite(LDO_PWR_EN_PIN, LOW)

#define DHTPIN 18     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11 // DHT11
#define BUZZPIN 5
#define LDO_PWR_EN_PIN 19 //AP2112K enable pin
#define I2C_SDA 26
#define I2C_SCL 27

TFT_eSPI tft = TFT_eSPI();
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SGP30 sgp;

int list_1[50] = {0};
int list_1_length = 20;
int list_2[50] = {0};
int list_2_length = 20;
int interval = 3000;

void setup()
{
    Serial.begin(115200);
    Serial.println("Enviroment Test!");

    dht.begin();

    pinMode(BUZZPIN, OUTPUT);
    pinMode(LDO_PWR_EN_PIN, OUTPUT);

    BUZZER_ON;
    LDO_1V8_ON;
    delay(1000);
    BUZZER_OFF;

    Wire.begin(I2C_SDA,I2C_SCL);

    Serial.println("SGP30 test");

    if (!sgp.begin())
    {
        Serial.println("Sensor not found :(");
        while (1)
            ;
    }

    //Draw frame
    tft.init();
    tft.fillScreen(TFT_BLACK);

    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);

    tft.setCursor(10, 40);
    tft.println("Makerfabs line chart");

    delay(1000);
    tft.fillScreen(TFT_BLACK);
    draw_line_chart_window("TVOC Line Chart ( Unit : ppb ) ", (String) "Interval : " + interval + " ms", list_1_length, 0, 1200, TFT_WHITE);
    draw_line_chart_window2("ECO2 Line Chart ( Unit : ppm ) ", (String) "Interval : " + interval + " ms", list_2_length, 0, 5000, TFT_WHITE);
}

void loop()
{
    draw_line_chart(list_1, list_1_length, 0, 1200, TFT_WHITE);
    draw_line_chart2(list_2, list_2_length, 0, 5000, TFT_WHITE);
    delay(interval);

    int tvoc = 0;
    int eco2 = 0;
    weather_read(&tvoc, &eco2);

    draw_line_chart(list_1, list_1_length, 0, 1200, TFT_BLACK);
    draw_line_chart2(list_2, list_2_length, 0, 5000, TFT_BLACK);
    add_list(list_1, list_1_length, tvoc);
    add_list(list_2, list_2_length, eco2);
}

// Add a num to the beginning of the list.
void add_list(int *list, int length, int num)
{
    for (int i = length - 2; i >= 0; i--)
    {
        *(list + i + 1) = *(list + i);
    }
    *list = num;
}

void draw_line_chart_window(String text1, String text2, int length, int low, int high, int color)
{
    //draw rect and unit
    //tft.drawRect(20, 20, 280, 200, color);
    tft.drawLine(30, 20, 30, 220, color);
    tft.drawLine(30, 220, 300, 220, color);

    tft.drawLine(20, 20, 30, 20, color);
    tft.drawLine(20, 120, 30, 120, color);

    tft.setTextColor(color);
    tft.setTextSize(1);

    tft.setCursor(0, 10);
    tft.println(high);

    tft.setCursor(80, 10);
    tft.println(text1);

    tft.setCursor(0, 110);
    tft.println((high + low) / 2);

    tft.setCursor(0, 210);
    tft.println(low);

    tft.setCursor(80, 230);
    tft.println(text2);

    int x_start = 32;
    int x_unit = 280 / (length - 1);
    for (int i = 0; i < length; i++)
    {
        int x = x_start + x_unit * i;
        if (i != 0 && i != length - 1)
            tft.drawLine(x, 220, x, 225, color);
    }
}

void draw_line_chart(int *list, int length, int low, int high, int color)
{
    //list to position
    int pos[50][2] = {0};
    int detail = 50;
    int x_start = 32;
    int y_start = 218;
    int x_unit = 280 / (length - 1);
    int y_unit = -200 / (detail - 1);
    for (int i = 0; i < length; i++)
    {
        pos[i][0] = x_start + i * x_unit;
        int y = map(*(list + i), low, high, 0, detail);
        if (y > detail)
            y = detail;
        pos[i][1] = y_start + y_unit * y;
    }

    //draw line chart
    for (int i = 0; i < length - 1; i++)
    {
        tft.drawLine(pos[i][0], pos[i][1], pos[i + 1][0], pos[i + 1][1], color);
    }
}

uint32_t getAbsoluteHumidity(float temperature, float humidity)
{
    // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
    const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]
    const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity);                                                                // [mg/m^3]
    return absoluteHumidityScaled;
}

void weather_read(int *tvoc, int *eco2)
{

    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    Serial.print("humidity=");
    Serial.println(humidity);
    Serial.print("temperature=");
    Serial.println(temperature);

    sgp.setHumidity(getAbsoluteHumidity(temperature, humidity));

    if (!sgp.IAQmeasure())
    {
        Serial.println("Measurement failed");
        return;
    }
    Serial.print("TVOC ");
    Serial.print(*tvoc = sgp.TVOC);
    Serial.print(" ppb\t");
    Serial.print("eCO2 ");
    Serial.print(*eco2 = sgp.eCO2);
    Serial.println(" ppm");
}

void draw_line_chart_window2(String text1, String text2, int length, int low, int high, int color)
{
    //draw rect and unit
    //tft.drawRect(20, 20, 280, 200, color);
    tft.drawLine(30, 260, 30, 460, color);
    tft.drawLine(30, 460, 300, 460, color);

    tft.drawLine(20, 260, 30, 260, color);
    tft.drawLine(20, 360, 30, 360, color);

    tft.setTextColor(color);
    tft.setTextSize(1);

    tft.setCursor(0, 250);
    tft.println(high);

    tft.setCursor(80, 250);
    tft.println(text1);

    tft.setCursor(0, 350);
    tft.println((high + low) / 2);

    tft.setCursor(0, 450);
    tft.println(low);

    tft.setCursor(80, 470);
    tft.println(text2);

    int x_start = 32;
    int x_unit = 280 / (length - 1);
    for (int i = 0; i < length; i++)
    {
        int x = x_start + x_unit * i;
        if (i != 0 && i != length - 1)
            tft.drawLine(x, 460, x, 465, color);
    }
}

void draw_line_chart2(int *list, int length, int low, int high, int color)
{
    //list to position
    int pos[50][2] = {0};
    int detail = 50;
    int x_start = 32;
    int y_start = 458;
    int x_unit = 280 / (length - 1);
    int y_unit = -200 / (detail - 1);
    for (int i = 0; i < length; i++)
    {
        pos[i][0] = x_start + i * x_unit;
        int y = map(*(list + i), low, high, 0, detail);
        if (y > detail)
            y = detail;
        pos[i][1] = y_start + y_unit * y;
    }

    //draw line chart
    for (int i = 0; i < length - 1; i++)
    {
        tft.drawLine(pos[i][0], pos[i][1], pos[i + 1][0], pos[i + 1][1], color);
    }
}
