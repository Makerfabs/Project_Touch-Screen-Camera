#include <SPI.h>
#include <SD.h>
#include <FS.h>
#include <esp_camera.h>
#include <HTTPClient.h>
#include <LovyanGFX.hpp>

#include "makerfabs_pin.h"
#define CAMERA_MODEL_MAKERFABS
#include "camera_pins.h"
//#define WIFI_MODE

#define ARRAY_LENGTH 320 * 240 * 3
#define SCRENN_ROTATION 3

/*
!!!MAKE SURE CHOICE RIGHT TOUCH SCREEN DRIVER!!!

You can only use one type:Resistive or Capacitive.

!!!MAKE SURE CHOICE RIGHT TOUCH SCREEN DRIVER!!!
*/
// #define NS2009_TOUCH  //Resistive screen driver
#define FT6236_TOUCH // Capacitive screen driver

#ifdef NS2009_TOUCH
#include "NS2009.h"
const int i2c_touch_addr = NS2009_ADDR;
#define get_pos ns2009_pos
#endif

#ifdef FT6236_TOUCH
#include "FT6236.h"
const int i2c_touch_addr = TOUCH_I2C_ADD;
#define get_pos ft6236_pos
#endif

// SPI control
#define SPI_ON_TFT digitalWrite(ESP32_TSC_9488_LCD_CS, LOW)
#define SPI_OFF_TFT digitalWrite(ESP32_TSC_9488_LCD_CS, HIGH)
#define SPI_ON_SD digitalWrite(ESP32_TSC_9488_SD_CS, LOW)
#define SPI_OFF_SD digitalWrite(ESP32_TSC_9488_SD_CS, HIGH)

struct LGFX_Config
{
    static constexpr spi_host_device_t spi_host = ESP32_TSC_9488_LCD_SPI_HOST;
    static constexpr int dma_channel = 1;
    static constexpr int spi_sclk = ESP32_TSC_9488_LCD_SCK;
    static constexpr int spi_mosi = ESP32_TSC_9488_LCD_MOSI;
    static constexpr int spi_miso = ESP32_TSC_9488_LCD_MISO;
};

static lgfx::LGFX_SPI<LGFX_Config> tft;
static LGFX_Sprite sprite(&tft);
static lgfx::Panel_ILI9488 panel;

const uint8_t img_rgb888_320_240_head[54] = {
    0x42, 0x4d, 0x36, 0x84, 0x3, 0x0, 0x0, 0x0, 0x0, 0x0, 0x36, 0x0, 0x0, 0x0, 0x28, 0x0,
    0x0, 0x0, 0x40, 0x1, 0x0, 0x0, 0xf0, 0x0, 0x0, 0x0, 0x1, 0x0, 0x18, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x84, 0x3, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

String imgname = "";
int img_index = 0;
int pos[2] = {0, 0};
int stream_flag = 1;

void setup()
{
    esp32_init();
    camera_init();
}

void loop()
{
    camera_fb_t *fb = NULL;
    fb = esp_camera_fb_get();

    if (stream_flag == 1)
    {
        tft.pushImage(0, 0, fb->width, fb->height, (lgfx::swap565_t *)fb->buf);
    }

    get_pos(pos);
    pos_rotation(pos, SCRENN_ROTATION);

    if (320 < pos[0] && pos[0] < 480)
    {
        if (0 < pos[1] && pos[1] < 80)
        {
            if (stream_flag == 1)
            {
                void *ptrVal = NULL;
                ptrVal = heap_caps_malloc(ARRAY_LENGTH, MALLOC_CAP_SPIRAM);
                uint8_t *rgb = (uint8_t *)ptrVal;
                fmt2rgb888(fb->buf, fb->len, PIXFORMAT_RGB565, rgb);
                if (save_image(SD, rgb) == -1)
                {
                    show_log(5);
                }
                else
                    show_log(0);
                heap_caps_free(ptrVal);
                rgb = NULL;
            }
            else
            {
                show_log(3);
            }
        }

        if (pos[1] > 80 && pos[1] < 160)
        {
            show_log(1);
            stream_flag = 0;
            tft.fillRect(0, 0, 320, 240, TFT_BLACK);
            print_img(SD, imgname, 320, 240);

#ifdef WIFI_MODE
            TestPostFileStream(imgname);
            show_log(4);
#endif
        }

        if (pos[1] < 240 && pos[1] > 160)
        {
            stream_flag = 1;
            show_log(2);
        }
    }
    esp_camera_fb_return(fb);
}

// ILI9488 init and SD card init
void esp32_init()
{
    int SD_init_flag = 0;
    Serial.begin(115200);
    Serial.println("ILI9488 Test!");

    // I2C init
    Wire.begin(ESP32_TSC_9488_I2C_SDA, ESP32_TSC_9488_I2C_SCL);
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

    // SPI init
    pinMode(ESP32_TSC_9488_SD_CS, OUTPUT);
    pinMode(ESP32_TSC_9488_LCD_CS, OUTPUT);
    SPI_OFF_SD;
    SPI_OFF_TFT;

    SPI.begin(ESP32_TSC_9488_HSPI_SCK, ESP32_TSC_9488_HSPI_MISO, ESP32_TSC_9488_HSPI_MOSI);

    // SD(SPI) init
    SPI_ON_SD;
    if (!SD.begin(ESP32_TSC_9488_SD_CS, SPI, 40000000))
    {
        Serial.println("Card Mount Failed");
        SD_init_flag = 1;
    }
    else
    {
        Serial.println("Card Mount Successed");
    }
    SPI_OFF_SD;

    Serial.println("SD init over.");

    // TFT(SPI) init
    SPI_ON_TFT;
    set_tft();
    tft.begin();
    tft.setRotation(SCRENN_ROTATION);
    tft.fillScreen(TFT_RED);
    delay(1500);
    tft.fillScreen(TFT_GREEN);
    delay(1500);
    tft.fillScreen(TFT_WHITE);
    delay(1500);
    tft.fillScreen(TFT_BLUE);
    delay(1500);
    tft.fillScreen(TFT_BLACK);
    delay(1500);

    // if SD init failed
    if (SD_init_flag == 1)
    {
        tft.setTextColor(TFT_RED);
        tft.setTextSize(3);
        tft.setCursor(10, 10);
        tft.println("  SD card initialization failed, please re-insert.");
        tft.setCursor(10, 60);
        tft.println("  Or touch the screen using only the camera (showing only the output stream)");

        tft.fillRect(100, 160, 280, 160, TFT_BLUE);
        tft.setCursor(130, 190);
        tft.println("    TOUCH");
        tft.setCursor(140, 220);
        tft.println("     TO");
        tft.setCursor(120, 250);
        tft.println("   CONTINUE");

        int pos[2] = {0, 0};

        while (1)
        {
            get_pos(pos);

            if (pos[0] > 100 && pos[0] < 380 && pos[1] > 160 && pos[1] < 320)
                break;
            delay(100);
        }
    }
    else
    {
        tft.setRotation(1);

        SPI_OFF_TFT;
        delay(10);
        print_img(SD, "/logo.bmp", 480, 320);
        SPI_ON_TFT;

        delay(1000);
        tft.setRotation(3);
    }

    draw_button();
    SPI_OFF_TFT;
    Serial.println("TFT init over.");

#ifdef WIFI_MODE
    // Trying to solve pn open, type:2 0 error with this line:
    WiFi.mode(WIFI_OFF);

    for (uint8_t t = 4; t > 0; t--)
    {
        Serial.printf("[SETUP] WAIT %d...\n", t);
        Serial.flush();
        delay(200);
    }

    Serial.println("Connecting to WIFI");
    // wait for WiFi connection
    WiFi.mode(WIFI_STA);
    WiFi.begin("Makerfabs", "20160704");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(50);
    }
    Serial.println("\nWIFI CONNECTED");
#endif
}

// Camera setting
void camera_init()
{
    // camera config
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;

    config.pixel_format = PIXFORMAT_RGB565;
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 10;
    config.fb_count = 1;

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        Serial.printf("Camera init failed with error 0x%x", err);
        while (1)
            ;
    }

    sensor_t *s = esp_camera_sensor_get();
    // initial sensors are flipped vertically and colors are a bit saturated
    if (s->id.PID == OV2640_PID)
    {
        s->set_vflip(s, 0);   // vertical flip
        s->set_hmirror(s, 0); // Horizontal mirror

        s->set_brightness(s, 0); // up the blightness just a bit
        s->set_saturation(s, 1); // lower the saturation
    }
    // drop down frame size for higher initial frame rate
    s->set_framesize(s, FRAMESIZE_QVGA);

    show_log(2);
}

// Save image to SD card
int save_image(fs::FS &fs, uint8_t *rgb)
{
    SPI_ON_SD;
    imgname = "/" + String(img_index) + "write.bmp";
    img_index++;

    Serial.println("Image name：" + imgname);

    File f = fs.open(imgname, "w");
    if (!f)
    {
        Serial.println("Failed to open file for writing");
        return -1;
    }

    f.write(img_rgb888_320_240_head, 54);
    f.write(rgb, 230400);

    f.close();
    SPI_OFF_SD;
    return 0;
}

void draw_button()
{
    SPI_ON_TFT;
    tft.fillScreen(TFT_BLACK);
    tft.fillRect(330, 0, 150, 70, TFT_BLUE);
    tft.fillRect(330, 80, 150, 70, TFT_BLUE);
    tft.fillRect(330, 160, 150, 70, TFT_BLUE);

    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(1);
    tft.setCursor(340, 30);
    tft.println("TAKE PHOTO");
    tft.setCursor(340, 110);
    tft.println("LAST PHOTO");
    tft.setCursor(340, 190);
    tft.println("START STREAM");

    SPI_OFF_TFT;
}

// Display image from file
int print_img(fs::FS &fs, String filename, int x, int y)
{
    SPI_ON_SD;
    File f = fs.open(filename, "r");
    if (!f)
    {
        Serial.println("Failed to open file for reading");
        f.close();
        return 0;
    }

    f.seek(54);
    int X = x;
    int Y = y;
    uint8_t RGB[3 * X];
    for (int row = 0; row < Y; row++)
    {
        f.seek(54 + 3 * X * row);
        f.read(RGB, 3 * X);
        SPI_OFF_SD;
        tft.pushImage(0, row, X, 1, (lgfx::rgb888_t *)RGB);
        SPI_ON_SD;
    }

    f.close();
    SPI_OFF_SD;
    return 0;
}

void show_log(int cmd_type)
{
    SPI_ON_TFT;
    tft.fillRect(0, 240, 480, 80, TFT_BLACK);

    tft.setTextColor(TFT_RED);
    tft.setTextSize(2);
    tft.setCursor(30, 280);

    switch (cmd_type)
    {
    case 0:
        tft.println("TAKE PHOTO");
        tft.println(imgname);
        break;

    case 1:
        tft.println("Show last photo");
        tft.println(imgname);
        break;

    case 2:
        tft.println("STREAMING");
        break;

    case 3:
        tft.println("NEED STREAM");
        break;

    case 4:
        tft.println("Show last photo");
        tft.println(imgname);
        tft.println("Upload success");
        break;
    case 5:
        tft.println("Write sd card wrong");
        break;

    default:
        break;
    }
    SPI_OFF_TFT;
}

#ifdef WIFI_MODE
void TestPostFileStream(String file_name)
{
    SPI_ON_SD;
    HTTPClient http;
    String filename = String(img_index - 1) + "write.bmp";
    String pathname = file_name;
    String archDomainAddress = "http://192.168.1.128:5002/json";

    http.begin(archDomainAddress);
    http.addHeader("Content-Type", "text/plain");

    File payloadFile = SD.open(pathname, FILE_READ);
    if (payloadFile)
    {
        Serial.println("File exists, starting POST");

        http.addHeader("File-Name", filename);
        int httpCode = http.sendRequest("POST", &payloadFile, payloadFile.size());

        if (httpCode > 0)
        {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTP] POST... code: %d\n", httpCode);

            // file found at server
            if (httpCode >= 200 && httpCode < 300)
            {
                String payload = http.getString();
                Serial.println("SUCCESS! DATA RECEIVED: ");
                Serial.println(payload);
            }
        }
        else
        {
            Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
    }
    else
    {
        Serial.println("File does not exist!");
    }
    payloadFile.close();
    http.end();
    SPI_OFF_SD;
    Serial.println("POST over");
}
#endif

void set_tft()
{
    panel.freq_write = 40000000;
    panel.freq_fill = 40000000;
    panel.freq_read = 16000000;

    panel.spi_cs = ESP32_TSC_9488_LCD_CS;
    panel.spi_dc = ESP32_TSC_9488_LCD_DC;
    panel.gpio_rst = ESP32_TSC_9488_LCD_RST;
    panel.gpio_bl = ESP32_TSC_9488_LCD_BL;

    tft.setPanel(&panel);
}

// transform touch screen pos
void pos_rotation(int pos[2], int rotation)
{
    if (pos[0] == -1)
        return;
    if (rotation == 0)
    {
        return;
    }
    if (rotation == 3)
    {
        int tempx = 480 - pos[1];
        int tempy = pos[0];
        pos[0] = tempx;
        pos[1] = tempy;
    }
}
