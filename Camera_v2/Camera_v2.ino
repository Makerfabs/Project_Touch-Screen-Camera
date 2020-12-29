#include "SPI.h"
#include "SD.h"
#include "FS.h"
#include "esp_camera.h"
#define CAMERA_MODEL_MAKERFABS
#include "camera_pins.h"
#include "HTTPClient.h"

#include <LovyanGFX.hpp>
#include "makerfabs_pin.h"

//If undefine debug, will be faster
//#define SERIAL_DEBUG
//#define WIFI_MODE

#define ARRAY_LENGTH 320 * 240 * 3

//#define NS2009_TOUCH //Resistive screen driver
#define FT6236_TOUCH //Capacitive screen driver

#define SCRENN_ROTATION 3

#ifdef NS2009_TOUCH
#include "NS2009.h"
const int i2c_touch_addr = NS2009_ADDR;
#endif

#ifdef FT6236_TOUCH
#include "FT6236.h"
const int i2c_touch_addr = TOUCH_I2C_ADD;
#endif

//SPI control
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
#ifdef SERIAL_DEBUG
    unsigned long start = micros();
#endif

    camera_fb_t *fb = NULL;

    fb = esp_camera_fb_get();

    if (stream_flag == 1)
    {
#ifdef SERIAL_DEBUG
        Serial.println(micros() - start);
#endif
        //Display rgb565 picture
        drawRGBBitmap(fb->buf, fb->width, fb->height);

#ifdef SERIAL_DEBUG
        Serial.println(micros() - start);
#endif
    }

#ifdef NS2009_TOUCH
    ns2009_pos(pos);
#endif
#ifdef FT6236_TOUCH
    ft6236_pos(pos);
#endif

#ifdef SERIAL_DEBUG
    String pos_str = (String)pos[0] + "," + (String)pos[1] + "," + (String)ns2009_get_press();
    Serial.println(pos_str);
#endif
    pos_rotation(pos, SCRENN_ROTATION);

    if (320 < pos[0] && pos[0] < 480)
    {
        if (0 < pos[1] && pos[1] < 80)
        {
            if (stream_flag == 1)
            {

#ifdef SERIAL_DEBUG
                Serial.println("Take a photo");
#endif

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

#ifdef SERIAL_DEBUG
                Serial.println("Please start steam");
#endif
                show_log(3);
            }
        }

        if (pos[1] > 80 && pos[1] < 160)
        {
#ifdef SERIAL_DEBUG
            Serial.println("Last photo:");
            Serial.println(imgname);
#endif
            show_log(1);
            stream_flag = 0;
            tft.fillRect(0, 0, 320, 240, TFT_BLACK);
            print_img(SD, imgname);

#ifdef WIFI_MODE
            TestPostFileStream(imgname);
            show_log(4);
#endif
        }

        if (pos[1] < 240 && pos[1] > 160)
        {
#ifdef SERIAL_DEBUG
            Serial.println("START STREAM");
#endif
            stream_flag = 1;
            show_log(2);
        }
    }

    /*
    if (pos[0] < 320 && pos[0] > 220)
    {
        if (pos[1] < 480 && pos[1] > 440)
        {
#ifdef SERIAL_DEBUG
            Serial.println("START STREAM");
#endif
            stream_flag = 0;
            testFillScreen();
        }
    }
    */

    esp_camera_fb_return(fb);

#ifdef SERIAL_DEBUG
    Serial.println(micros() - start);
    Serial.println("over a circle");
#endif
}

//ILI9488 init and SD card init
void esp32_init()
{
    int SD_init_flag = 0;
    Serial.begin(115200);
    Serial.println("ILI9488 Test!");

    //I2C init
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

    //SPI init
    pinMode(ESP32_TSC_9488_SD_CS, OUTPUT);
    pinMode(ESP32_TSC_9488_LCD_CS, OUTPUT);
    SPI_OFF_SD;
    SPI_OFF_TFT;

    SPI.begin(ESP32_TSC_9488_HSPI_SCK, ESP32_TSC_9488_HSPI_MISO, ESP32_TSC_9488_HSPI_MOSI);

    //SD(SPI) init
    SPI_ON_SD;
    if (!SD.begin(ESP32_TSC_9488_SD_CS, SPI, 60000000))
    {
        Serial.println("Card Mount Failed");
        SD_init_flag = 1;

        //while (1);
    }
    else
    {
        Serial.println("Card Mount Successed");
    }
    SPI_OFF_SD;

    Serial.println("SD init over.");

    //TFT(SPI) init
    SPI_ON_TFT;
    set_tft();
    tft.begin();
    tft.setRotation(SCRENN_ROTATION);
    tft.fillScreen(TFT_BLACK);

    //if SD init failed
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
#ifdef NS2009_TOUCH
            ns2009_pos(pos);
#endif
#ifdef FT6236_TOUCH
            ft6236_pos(pos);
#endif

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
        print_logo_img(SD, "/logo.bmp");
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

//Camera setting
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
    //initial sensors are flipped vertically and colors are a bit saturated
    if (s->id.PID == OV2640_PID)
    {
        s->set_vflip(s, 1);      //flip it back
        s->set_brightness(s, 0); //up the blightness just a bit
        s->set_saturation(s, 1); //lower the saturation
    }
    //drop down frame size for higher initial frame rate
    s->set_framesize(s, FRAMESIZE_QVGA);

    show_log(2);
}

//Save image to SD card
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
    //tft.fillRect(220, 440, 100, 40, TFT_GREEN);

    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(1);
    tft.setCursor(340, 30);
    tft.println("TAKE PHOTO");
    tft.setCursor(340, 110);
    tft.println("LAST PHOTO");
    tft.setCursor(340, 190);
    tft.println("START STREAM");
    //tft.setCursor(230, 450);
    //tft.println("TEST");

    SPI_OFF_TFT;
}

//Display image from file
int print_img(fs::FS &fs, String filename)
{
    SPI_ON_SD;
    File f = fs.open(filename, "r");
    if (!f)
    {
        Serial.println("Failed to open file for reading");
        return 0;
    }

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
            tft.drawPixel(col, row, tft.color565(RGB[col * 3 + 2], RGB[col * 3 + 1], RGB[col * 3]));
        }
        SPI_OFF_TFT;
        SPI_ON_SD;
    }

    f.close();
    SPI_OFF_SD;
    return 0;
}

int print_logo_img(fs::FS &fs, String filename)
{
    SPI_ON_SD;
    File f = fs.open(filename);
    if (!f)
    {
        Serial.println("Failed to open file for reading");
        return 0;
    }
    else
    {
        Serial.println("Success open file for reading");
    }

    f.seek(54);
    int X = 480;
    int Y = 320;
    uint8_t RGB[3 * X];
    for (int row = 0; row < Y; row++)
    {
        f.seek(54 + 3 * X * row);
        f.read(RGB, 3 * X);
        SPI_OFF_SD;
        SPI_ON_TFT;
        for (int col = 0; col < X; col++)
        {
            tft.drawPixel(col, row, tft.color565(RGB[col * 3 + 2], RGB[col * 3 + 1], RGB[col * 3]));
        }
        SPI_OFF_TFT;
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

void drawRGBBitmap(uint8_t *bitmap, int16_t w, int16_t h)
{
    SPI_ON_TFT;
    tft.startWrite();
    for (int16_t j = 0; j < h; j++)
    {
        for (int16_t i = 0; i < w; i++)
        {
            uint16_t temp = bitmap[j * w * 2 + 2 * i] * 256 + bitmap[j * w * 2 + 2 * i + 1];
            tft.writePixel(i, j, temp);
        }
    }
    tft.endWrite();
    SPI_OFF_TFT;
}

unsigned long testFillScreen()
{
    SPI_ON_TFT;
    unsigned long start = micros();
    tft.fillRect(0, 0, 320, 240, TFT_YELLOW);
    tft.fillRect(0, 0, 320, 240, TFT_RED);
    tft.fillRect(0, 0, 320, 240, TFT_GREEN);
    tft.fillRect(0, 0, 320, 240, TFT_BLUE);
    tft.fillRect(0, 0, 320, 240, TFT_BLACK);
    start = micros() - start;
    tft.fillRect(0, 340, 220, 140, TFT_BLACK);
    tft.setCursor(0, 360);
    tft.println("cost:" + (String)start);
    return micros() - start;
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
    // パネルクラスに各種設定値を代入していきます。
    // （LCD一体型製品のパネルクラスを選択した場合は、
    //   製品に合った初期値が設定されているので設定は不要です）

    // 通常動作時のSPIクロックを設定します。
    // ESP32のSPIは80MHzを整数で割った値のみ使用可能です。
    // 設定した値に一番近い設定可能な値が使用されます。
    panel.freq_write = 60000000;
    //panel.freq_write = 20000000;

    // 単色の塗り潰し処理時のSPIクロックを設定します。
    // 基本的にはfreq_writeと同じ値を設定しますが、
    // より高い値を設定しても動作する場合があります。
    panel.freq_fill = 60000000;
    //panel.freq_fill  = 27000000;

    // LCDから画素データを読取る際のSPIクロックを設定します。
    panel.freq_read = 16000000;

    // SPI通信モードを0~3から設定します。
    panel.spi_mode = 0;

    // データ読み取り時のSPI通信モードを0~3から設定します。
    panel.spi_mode_read = 0;

    // 画素読出し時のダミービット数を設定します。
    // 画素読出しでビットずれが起きる場合に調整してください。
    panel.len_dummy_read_pixel = 8;

    // データの読取りが可能なパネルの場合はtrueを、不可の場合はfalseを設定します。
    // 省略時はtrueになります。
    panel.spi_read = true;

    // データの読取りMOSIピンで行うパネルの場合はtrueを設定します。
    // 省略時はfalseになります。
    panel.spi_3wire = false;

    // LCDのCSを接続したピン番号を設定します。
    // 使わない場合は省略するか-1を設定します。
    panel.spi_cs = ESP32_TSC_9488_LCD_CS;

    // LCDのDCを接続したピン番号を設定します。
    panel.spi_dc = ESP32_TSC_9488_LCD_DC;

    // LCDのRSTを接続したピン番号を設定します。
    // 使わない場合は省略するか-1を設定します。
    panel.gpio_rst = ESP32_TSC_9488_LCD_RST;

    // LCDのバックライトを接続したピン番号を設定します。
    // 使わない場合は省略するか-1を設定します。
    panel.gpio_bl = ESP32_TSC_9488_LCD_BL;

    // バックライト使用時、輝度制御に使用するPWMチャンネル番号を設定します。
    // PWM輝度制御を使わない場合は省略するか-1を設定します。
    panel.pwm_ch_bl = -1;

    // バックライト点灯時の出力レベルがローかハイかを設定します。
    // 省略時は true。true=HIGHで点灯 / false=LOWで点灯になります。
    panel.backlight_level = true;

    // invertDisplayの初期値を設定します。trueを設定すると反転します。
    // 省略時は false。画面の色が反転している場合は設定を変更してください。
    panel.invert = false;

    // パネルの色順がを設定します。  RGB=true / BGR=false
    // 省略時はfalse。赤と青が入れ替わっている場合は設定を変更してください。
    panel.rgb_order = false;

    // パネルのメモリが持っているピクセル数（幅と高さ）を設定します。
    // 設定が合っていない場合、setRotationを使用した際の座標がずれます。
    // （例：ST7735は 132x162 / 128x160 / 132x132 の３通りが存在します）
    panel.memory_width = ESP32_TSC_9488_LCD_WIDTH;
    panel.memory_height = ESP32_TSC_9488_LCD_HEIGHT;

    // パネルの実際のピクセル数（幅と高さ）を設定します。
    // 省略時はパネルクラスのデフォルト値が使用されます。
    panel.panel_width = ESP32_TSC_9488_LCD_WIDTH;
    panel.panel_height = ESP32_TSC_9488_LCD_HEIGHT;

    // パネルのオフセット量を設定します。
    // 省略時はパネルクラスのデフォルト値が使用されます。
    panel.offset_x = 0;
    panel.offset_y = 0;

    // setRotationの初期化直後の値を設定します。
    panel.rotation = 0;

    // setRotationを使用した時の向きを変更したい場合、offset_rotationを設定します。
    // setRotation(0)での向きを 1の時の向きにしたい場合、 1を設定します。
    panel.offset_rotation = 0;

    // 設定を終えたら、LGFXのsetPanel関数でパネルのポインタを渡します。
    tft.setPanel(&panel);
}

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
