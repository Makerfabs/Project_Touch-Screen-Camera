#include "SPI.h"
#include "ILI9488.h"
//#include "dragon.h"
#include "esp_camera.h"

#define CAMERA_MODEL_MAKERFABS
#include "camera_pins.h"

#define TFT_CS 15
#define TFT_DC 33
#define TFT_LED -1
#define TFT_RST -1

//SPI
#define SPI_MOSI 13
#define SPI_MISO 12
#define SPI_SCK 14

// If using the breakout, change pins as desired
//ILI9488 tft = ILI9488(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
ILI9488 tft = ILI9488(TFT_CS, TFT_DC, TFT_RST);

//unsigned short temp[153600];
//unsigned short temp[76800];
//unsigned short temp[100];

void setup()
{
    Serial.begin(115200);
    Serial.println("ILI9488 Test!");

    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

    tft.begin();

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
    //config.pixel_format = PIXFORMAT_JPEG;
    config.pixel_format = PIXFORMAT_RGB565;
    //init with high specs to pre-allocate larger buffers
    if (psramFound())
    {
        //config.frame_size = FRAMESIZE_UXGA;
        config.frame_size = FRAMESIZE_QVGA;
        config.jpeg_quality = 10;
        config.fb_count = 2;
    }
    else
    {
        //config.frame_size = FRAMESIZE_SVGA;
        config.frame_size = FRAMESIZE_QVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }
    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        Serial.printf("Camera init failed with error 0x%x", err);
        while (1)
            ;
    }
    Serial.printf("Camera init Success");

    sensor_t *s = esp_camera_sensor_get();
    //initial sensors are flipped vertically and colors are a bit saturated
    if (s->id.PID == OV3660_PID)
    {
        s->set_vflip(s, 1);       //flip it back
        s->set_brightness(s, 1);  //up the blightness just a bit
        s->set_saturation(s, -2); //lower the saturation
    }
    //drop down frame size for higher initial frame rate
    s->set_framesize(s, FRAMESIZE_QVGA);

#if defined(CAMERA_MODEL_M5STACK_WIDE)
    s->set_vflip(s, 1);
    s->set_hmirror(s, 1);
#endif
    tft.setRotation(2);
}

void loop(void)
{
    //tft.setRotation(r);
    //tft.fillScreen(ILI9488_BLACK);

    camera_fb_t *fb = NULL;

    fb = esp_camera_fb_get();
    if (!fb)
    {
        Serial.println("Camera capture failed");
    }
    Serial.println(fb->len);

    tft.drawRGBBitmap(0, 0, (uint16_t *)fb->buf, fb->width, fb->height);
    //drawRGBBitmap(fb->buf, fb->width, fb->height);

    esp_camera_fb_return(fb);
    //tft.drawRGBBitmap(0, 0, (uint16_t *)viewBitmap, WIDTH, HEIGHT);

    //delay(3000);
}

void drawRGBBitmap(uint8_t *bitmap, int16_t w, int16_t h)
{
    tft.startWrite();
    for (int16_t j = 0; j < h; j++)
    {
        for (int16_t i = 0; i < w; i++)
        {
            uint16_t temp = 0;
            temp =bitmap[j * w * 2 + 2 * i] << 8 + bitmap[j * w * 2 + 2 * i + 1];
            tft.writePixel(i, j, temp);
        }
    }
    tft.endWrite();
}