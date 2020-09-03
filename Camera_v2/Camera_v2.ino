#include "SPI.h"
#include "SD.h"
#include "FS.h"
#include "esp_camera.h"
#define CAMERA_MODEL_MAKERFABS
#include "camera_pins.h"

#include <LovyanGFX.hpp>
#include "makerfabs_pin.h"

//#define SERIAL_DEBUG

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

struct LGFX_Config
{
    static constexpr spi_host_device_t spi_host = VSPI_HOST;
    static constexpr int dma_channel = 1;
    static constexpr int spi_sclk = LCD_SCK;
    static constexpr int spi_mosi = LCD_MOSI;
    static constexpr int spi_miso = LCD_MISO;
};

static lgfx::LGFX_SPI<LGFX_Config> tft;
static LGFX_Sprite sprite(&tft);
static lgfx::Panel_ILI9488 panel;

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
        //显示rgb888图像，rgb需要转码，用于读sd卡的24位真彩色bmp
        //print_RGB888_img(rgb);

        //将转化放到psram执行，速度没有改善，画质反而更差了
        /*
        void *ptrVal = NULL;
        ptrVal = heap_caps_malloc(153600, MALLOC_CAP_SPIRAM);
        uint16_t *u16bitmap = (uint16_t *)ptrVal;
        RGB_u8_u16(fb->buf, u16bitmap);
        //drawRGB16Bitmap(u16bitmap, fb->width, fb->height);
        tft.drawRGBBitmap(0, 0, u16bitmap, 320, 240);
        heap_caps_free(ptrVal);
        u16bitmap = NULL;
        */

        //显示rgb565图像
        drawRGBBitmap(fb->buf, fb->width, fb->height);

        //draw_fast_pic(fb->buf, fb->width, fb->height);
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
    String pos_str = (String)pos[0] + "," + (String)pos[1];
#ifdef SERIAL_DEBUG
    Serial.println(pos_str);
#endif

    if (0 < pos[0] && pos[0] < 100)
    {
        if (240 < pos[1] && pos[1] < 340)
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
                save_image(SD, rgb);
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
    }

    if (pos[0] < 210 && pos[0] > 110)
    {
        if (pos[1] < 340 && pos[1] > 240)
        {
#ifdef SERIAL_DEBUG
            Serial.println("Last photo:");
            Serial.println(imgname);
#endif
            show_log(1);
            stream_flag = 0;
            tft.fillRect(0, 0, 320, 240, TFT_BLACK);
            print_img(SD, imgname);
        }
    }

    if (pos[0] < 320 && pos[0] > 220)
    {
        if (pos[1] < 340 && pos[1] > 240)
        {
#ifdef SERIAL_DEBUG
            Serial.println("START STREAM");
#endif
            stream_flag = 1;
            show_log(2);
        }
    }

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

    esp_camera_fb_return(fb);

#ifdef SERIAL_DEBUG
    Serial.println(micros() - start);
    Serial.println("over a circle");
#endif
}

void esp32_init()
{
    Serial.begin(115200);
    Serial.println("ILI9488 Test!");

    //I2C init
    Wire.begin(I2C_SDA, I2C_SCL);
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
    if (!SD.begin(SD_CS, SPI, 80000000))
    {
        Serial.println("Card Mount Failed");
        while (1)
            ;
    }
    else
    {
        Serial.println("Card Mount Successed");
    }
    //sd_test();
    SPI_OFF_SD;

    Serial.println("SD init over.");

    //TFT(SPI) init
    SPI_ON_TFT;
       set_tft();
    tft.begin();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);
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
    tft.fillRect(0, 240, 100, 100, TFT_BLUE);
    tft.fillRect(110, 240, 100, 100, TFT_BLUE);
    tft.fillRect(220, 240, 100, 100, TFT_BLUE);
    tft.fillRect(220, 440, 100, 40, TFT_GREEN);

    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(1);
    tft.setCursor(10, 290);
    tft.println("TAKE PHOTO");
    tft.setCursor(120, 290);
    tft.println("LAST PHOTO");
    tft.setCursor(230, 290);
    tft.println("START STREAM");
    tft.setCursor(230, 450);
    tft.println("TEST");

    SPI_OFF_TFT;
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

void show_log(int cmd_type)
{
    SPI_ON_TFT;
    tft.fillRect(0, 340, 220, 140, TFT_BLACK);

    tft.setTextColor(TFT_RED);
    tft.setTextSize(2);
    tft.setCursor(0, 360);

    switch (cmd_type)
    {
    case 0:
        tft.println("TAKE PHOTO");
        tft.println(imgname);
        break;

    case 1:
        tft.println("Show las photo");
        tft.println(imgname);
        break;

    case 2:
        tft.println("STREAMING");
        break;

    case 3:
        tft.println("NEED STREAM");
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

void drawRGB16Bitmap(uint16_t *bitmap, int16_t w, int16_t h)
{
    tft.startWrite();
    for (int16_t j = 0; j < h; j++)
    {
        for (int16_t i = 0; i < w; i++)
        {
            tft.writePixel(i, j, bitmap[j * w + i]);
        }
    }
    tft.endWrite();
}

void RGB_u8_u16(uint8_t *src, uint16_t *out)
{
    for (long i = 0; i < 76800; i++)
    {
        out[i] = src[2 * i] * 256 + src[2 * i + 1];
    }
}

void draw_fast_pic(uint8_t *bitmap, int16_t w, int16_t h)
{
    tft.startWrite();
    for (int16_t j = 0; j < h; j += 2)
    {
        for (int16_t i = 0; i < w; i += 2)
        {
            uint16_t temp = bitmap[j * w * 2 + 2 * i] * 256 + bitmap[j * w * 2 + 2 * i + 1];
            tft.writePixel(i, j, temp);
        }
    }
    tft.endWrite();
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
  panel.spi_cs = LCD_CS;

  // LCDのDCを接続したピン番号を設定します。
  panel.spi_dc = LCD_DC;

  // LCDのRSTを接続したピン番号を設定します。
  // 使わない場合は省略するか-1を設定します。
  panel.gpio_rst = LCD_RST;

  // LCDのバックライトを接続したピン番号を設定します。
  // 使わない場合は省略するか-1を設定します。
  panel.gpio_bl = LCD_BL;

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
  panel.memory_width = LCD_WIDTH;
  panel.memory_height = LCD_HEIGHT;

  // パネルの実際のピクセル数（幅と高さ）を設定します。
  // 省略時はパネルクラスのデフォルト値が使用されます。
  panel.panel_width = LCD_WIDTH;
  panel.panel_height = LCD_HEIGHT;

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