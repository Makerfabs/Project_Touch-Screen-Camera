#include "SPI.h"
#include "ILI9488.h"
#include "SD.h"
#include "FS.h"
#include "esp_camera.h"
#define CAMERA_MODEL_MAKERFABS
#include "camera_pins.h"

#define ARRAY_LENGTH 320 * 240 * 3

//Choice your touch IC
#define ESP32_SDA 26
#define ESP32_SCL 27

//#define NS2009_TOUCH
#define FT6236_TOUCH

#ifdef NS2009_TOUCH
#include "NS2009.h"
const int i2c_touch_addr = NS2009_ADDR;
#endif

#ifdef FT6236_TOUCH
#include "FT6236.h"
const int i2c_touch_addr = TOUCH_I2C_ADD;
#endif

//SPI
#define TFT_CS 15
#define TFT_DC 33
#define TFT_LED -1
#define TFT_RST -1
#define SPI_MOSI 13
#define SPI_MISO 12
#define SPI_SCK 14

//SD Card
#define SD_CS 4

//SPI control
#define SPI_ON_TFT digitalWrite(TFT_CS, LOW)
#define SPI_OFF_TFT digitalWrite(TFT_CS, HIGH)
#define SPI_ON_SD digitalWrite(SD_CS, LOW)
#define SPI_OFF_SD digitalWrite(SD_CS, HIGH)

ILI9488 tft = ILI9488(TFT_CS, TFT_DC, TFT_RST);

const uint8_t img_rgb888_320_240_head[54] = {
    0x42, 0x4d, 0x36, 0x84, 0x3, 0x0, 0x0, 0x0, 0x0, 0x0, 0x36, 0x0, 0x0, 0x0, 0x28, 0x0,
    0x0, 0x0, 0x40, 0x1, 0x0, 0x0, 0xf0, 0x0, 0x0, 0x0, 0x1, 0x0, 0x18, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x84, 0x3, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

String file_list[20];
int file_num = 0;
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
    void *ptrVal = NULL;
    fb = esp_camera_fb_get();
    ptrVal = heap_caps_malloc(ARRAY_LENGTH, MALLOC_CAP_SPIRAM);
    uint8_t *rgb = (uint8_t *)ptrVal;

    if (stream_flag == 1)
    {
        fmt2rgb888(fb->buf, fb->len, PIXFORMAT_RGB565, rgb);
        print_RGB888_img(rgb);
    }

#ifdef NS2009_TOUCH
    ns2009_pos(pos);
#endif
#ifdef FT6236_TOUCH
    ft6236_pos(pos);
#endif
    String pos_str = (String)pos[0] + "," + (String)pos[1];
    Serial.println(pos_str);
    if (pos[0] < 100)
    {
        if (pos[1] < 340 && pos[1] > 240)
        {
            if (stream_flag == 1)
            {
                Serial.println("Take a photo");
                save_image(SD, rgb);
            }
            else{
                Serial.println("Please start steam");
            }
        }
    }

    if (pos[0] < 210 && pos[0] > 110)
    {
        if (pos[1] < 340 && pos[1] > 240)
        {
            Serial.println("Last photo:");
            Serial.println(imgname);
            stream_flag = 0;
            print_img(SD, imgname);
        }
    }

    if (pos[0] < 320 && pos[0] > 220)
    {
        if (pos[1] < 340 && pos[1] > 240)
        {
            Serial.println("START STREAM");
            stream_flag = 1;
        }
    }

    esp_camera_fb_return(fb);
    heap_caps_free(ptrVal);
    rgb = NULL;
}

void esp32_init()
{
    Serial.begin(115200);
    Serial.println("ILI9488 Test!");

    //I2C init
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

    //SPI init
    pinMode(SD_CS, OUTPUT);
    pinMode(TFT_CS, OUTPUT);
    SPI_OFF_SD;
    SPI_OFF_TFT;

    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

    //SD(SPI) init
    SPI_ON_SD;
    if (!SD.begin(SD_CS, SPI))
    {
        Serial.println("Card Mount Failed");
        while (1)
            ;
    }
    else
    {
        Serial.println("Card Mount Successed");
    }
    sd_test();
    SPI_OFF_SD;

    Serial.println("SD init over.");

    //TFT(SPI) init
    SPI_ON_TFT;
    tft.begin();
    tft.setRotation(0);
    tft.fillScreen(ILI9488_BLACK);
    draw_button();
    SPI_OFF_TFT;
    Serial.println("TFT init over.");
}

void camera_init()
{
    //camera config
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
    //initial sensors are flipped vertically and colors are a bit saturated
    if (s->id.PID == OV2640_PID)
    {
        s->set_vflip(s, 1);      //flip it back
        s->set_brightness(s, 0); //up the blightness just a bit
        s->set_saturation(s, 1); //lower the saturation
    }
    //drop down frame size for higher initial frame rate
    s->set_framesize(s, FRAMESIZE_QVGA);
}

void print_RGB888_img(uint8_t *rgb)
{
    SPI_ON_TFT;
    int X = 320;
    int Y = 240;
    for (int row = 0; row < Y; row++)
    {
        for (int col = 0; col < X; col++)
        {
            tft.drawPixel(col, row, tft.color565(rgb[320 * 3 * row + col * 3 + 2], rgb[320 * 3 * row + col * 3] + 1, rgb[320 * 3 * row + col * 3]));
        }
    }
    SPI_OFF_TFT;
}

void sd_test()
{
    SPI_ON_SD;
    //Read SD
    file_num = get_file_list(SD, "/", 0, file_list);
    Serial.print("File count:");
    Serial.println(file_num);
    Serial.println("All File:");
    for (int i = 0; i < file_num; i++)
    {
        Serial.println(file_list[i]);
    }

    Serial.println("SD Test!");
    SPI_OFF_SD;
}

int get_file_list(fs::FS &fs, const char *dirname, uint8_t levels, String filelist[30])
{
    Serial.printf("Listing directory: %s\n", dirname);
    int i = 0;

    File root = fs.open(dirname);
    if (!root)
    {
        Serial.println("Failed to open directory");
        return i;
    }
    if (!root.isDirectory())
    {
        Serial.println("Not a directory");
        return i;
    }

    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
        }
        else
        {
            String temp = file.name();

            filelist[i] = temp;
            i++;
        }
        file = root.openNextFile();
    }
    return i;
}

int save_image(fs::FS &fs, uint8_t *rgb)
{
    SPI_ON_SD;
    imgname = "/" + String(img_index) + "write.bmp";
    img_index++;
    Serial.println("Image name：" + imgname);
    //String imgname = "writetest.bmp";
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
    tft.fillRect(0, 240, 100, 100, ILI9488_BLUE);
    tft.fillRect(110, 240, 100, 100, ILI9488_BLUE);
    tft.fillRect(220, 240, 100, 100, ILI9488_BLUE);

    tft.setTextColor(ILI9488_WHITE);
    tft.setTextSize(1);
    tft.setCursor(10, 290);
    tft.println("TAKE PHOTO");
    tft.setCursor(120, 290);
    tft.println("LAST PHOTO");
    tft.setCursor(220, 290);
    tft.println("START STREAM");
}

int print_img(fs::FS &fs, String filename)
{
    SPI_ON_SD;
    File f = fs.open(filename, "r");
    if (!f)
    {
        Serial.println("Failed to open file for reading");
        return 0;
    }

    // 从54偏移位置开始读
    f.seek(54);
    int X = 320;
    int Y = 240;
    uint8_t RGB[3 * X];
    for (int row = 0; row < Y; row++)
    {
        f.seek(54 + 3 * X * row);
        f.read(RGB, 3 * X);
        SPI_OFF_SD;
        SPI_ON_TFT;
        for (int col = 0; col < X; col++)
        {
            tft.drawPixel(col, row, tft.color565(RGB[col * 3 + 2], RGB[col * 3] + 1, RGB[col * 3]));
        }
        SPI_OFF_TFT;
        SPI_ON_SD;
    }

    f.close();
    SPI_OFF_SD;
    return 0;
}
