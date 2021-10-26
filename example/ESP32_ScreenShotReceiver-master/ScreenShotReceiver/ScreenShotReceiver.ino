/*----------------------------------------------------------------------------/
  ESP32 ScreenShotReceiver  

Original Source:  
 https://github.com/lovyan03/ESP32_ScreenShotReceiver/  

Licence:  
 [MIT](https://github.com/lovyan03/ESP32_ScreenShotReceiver/blob/master/LICENSE)  

Author:  
 [lovyan03](https://twitter.com/lovyan03)  

/----------------------------------------------------------------------------*/

#if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_FIRE)
#include <M5StackUpdater.h> // https://github.com/tobozo/M5Stack-SD-Updater/
#endif

#include "makerfabs_pin.h"
#include <esp_wifi.h>

#include "src/TCPReceiver.h"

//WIFI
const char *ssid = "Makerfabs";
const char *password = "20160704";

struct LGFX_Config
{
  static constexpr spi_host_device_t spi_host = VSPI_HOST;

  static constexpr int dma_channel = 1;

  static constexpr int spi_sclk = LCD_SCK;
  static constexpr int spi_mosi = LCD_MOSI;
  static constexpr int spi_miso = LCD_MISO;
};

// 用意した設定用の構造体を、LGFX_SPIクラスにテンプレート引数として設定し、インスタンスを作成します。
static lgfx::LGFX_SPI<LGFX_Config> lcd;
static LGFX_Sprite sprite(&lcd);

// Panelクラスのインスタンスを作成します。使用するパネルにあったクラスを選択してください。
//static lgfx::Panel_ST7789 panel;
static lgfx::Panel_ILI9488 panel;

static TCPReceiver recv;

void setup(void)
{
  Serial.begin(115200);
  Serial.flush();

#if defined(__M5STACKUPDATER_H)
  M5.begin();
#ifdef __M5STACKUPDATER_H
  if (digitalRead(BUTTON_A_PIN) == 0)
  {
    Serial.println("Will Load menu binary");
    updateFromFS(SD);
    ESP.restart();
  }
#endif
#endif
  //lcd.begin();

  set_lcd();

  // SPIバスの初期化とパネルの初期化を実行すると使用可能になります。
  lcd.init();

  lcd.setColorDepth(24);
  lcd.setRotation(0);
  if (lcd.width() < lcd.height())
    lcd.setRotation(1);

  lcd.setFont(&fonts::Font2);

  Serial.println("WiFi begin.");

  lcd.println("WiFi begin.");

  //makerfabs WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" CONNECTED");

  Serial.println(String("IP:") + WiFi.localIP().toString());
  lcd.println(WiFi.localIP().toString());

  /*

  // 記憶しているAPへ接続試行
  WiFi.mode(WIFI_MODE_STA);
  WiFi.begin();

  // 接続できるまで10秒待機
  for (int i = 0; WiFi.status() != WL_CONNECTED && i < 100; i++)
  {
    delay(100);
  }

  // 接続できない場合はSmartConfigを起動
  // https://itunes.apple.com/app/id1071176700
  // https://play.google.com/store/apps/details?id=com.cmmakerclub.iot.esptouch
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.print("SmartConfig start.");
    lcd.println("SmartConfig start.");
    WiFi.mode(WIFI_MODE_APSTA);
    WiFi.beginSmartConfig();

    while (WiFi.status() != WL_CONNECTED)
    {
      delay(100);
    }
    WiFi.stopSmartConfig();
    WiFi.mode(WIFI_MODE_STA);
  }

  Serial.println(String("IP:") + WiFi.localIP().toString());
  lcd.println(WiFi.localIP().toString());

  */

  recv.setup(&lcd);
}

void loop(void)
{
  recv.loop();
}

void set_lcd()
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
  lcd.setPanel(&panel);
}