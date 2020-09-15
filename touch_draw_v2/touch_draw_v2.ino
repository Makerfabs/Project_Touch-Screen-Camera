#include <Wire.h>
#include "SPI.h"
#include <LovyanGFX.hpp>
#include "makerfabs_pin.h"

//Choice your touch IC
#define NS2009_TOUCH
//#define FT6236_TOUCH

#ifdef NS2009_TOUCH
#include "NS2009.h"
const int i2c_touch_addr = NS2009_ADDR;
#endif

#ifdef FT6236_TOUCH
#include "FT6236.h"
const int i2c_touch_addr = TOUCH_I2C_ADD;
#endif

//SPI control
#define SPI_ON_TFT digitalWrite(LCD_CS, LOW)
#define SPI_OFF_TFT digitalWrite(LCD_CS, HIGH)

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

int last_pos[2] = {0, 0};
int draw_color = TFT_WHITE;

void setup()
{

    Serial.begin(115200);
    while (!Serial)
        ; // Leonardo: wait for serial monitor
    Serial.println("\n NS2009 test");

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

    pinMode(LCD_CS, OUTPUT);
    SPI_OFF_TFT;
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

    //TFT(SPI) init
    SPI_ON_TFT;

    set_tft();
    tft.begin();
    //tft.init();
    tft.fillScreen(TFT_BLACK);
    tft.fillRect(0, 0, 80, 40, TFT_RED);
    tft.fillRect(80, 0, 80, 40, TFT_GREEN);
    tft.fillRect(160, 0, 80, 40, TFT_BLUE);
    tft.fillRect(240, 0, 80, 40, TFT_YELLOW);
    SPI_OFF_TFT;
}

void loop()
{

    int pos[2] = {0, 0};
#ifdef NS2009_TOUCH
    Serial.println(ns2009_pos(pos));
#endif
#ifdef FT6236_TOUCH
    ft6236_pos(pos);
#endif
    Serial.printf("%d,%d\n", pos[0], pos[1]);
    if (0 < pos[1] && pos[1] < 40)
    {
        if (0 < pos[0] && pos[0] < 80)
        {
            draw_color = TFT_RED;
        }
        else if (80 < pos[0] && pos[0] < 160)
        {
            draw_color = TFT_GREEN;
        }

        else if (160 < pos[0] && pos[0] < 240)
        {
            draw_color = TFT_BLUE;
        }
        else if (240 < pos[0] && pos[0] < 320)
        {
            draw_color = TFT_YELLOW;
        }
    }
    else
    {
        tft.fillRect(pos[0], pos[1], 3, 3, draw_color);
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