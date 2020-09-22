/*
-------------------------------------------------
titile  :   Makerfabs IO define
author  :   Vincent
create  :   2020/9/4
version :   1.1
update  :   2020/9/11

Note:
    v1.0 : Create Project.
    v1.1 : Add MakePython Audio support.

1.
For easy use ESP32 GPIO,define SPI, I2C, and LCD, SDcard pins.
And define some parameters, such as screen size.

2.
Sigillum:
MP = MakePython
MP_ESP32 = MakePython ESP32 SSD1306
MP_ESP32_COLOR = MakePython ESP32 Color LCD (ST7789)

TSC = Touch Screen Camera
ESP32_TSC_9488 = ESP32 Touch Screen Camera With ILI9488

3.
//#define ESP32_COLOR_7789 OK
"Ok" means tested and ready to use
Don't use "ERR" or "TEST"

4.
For the motherboard combination expansion board.
This is to be determined and may be modified.
A more prudent approach is to copy the pin value directly.

-------------------------------------------------
Support List:

    (1)MakePython:
        MakePython ESP32 COLOR LCD
        MakePython ESP32 SSD1306
        MakePython Audio

    (2)Touch Screen Camera:
        ESP32 Touch Screen Camera With 9488

-------------------------------------------------
*/

//BOARD

//#define MP_ESP32 OK
//#define MP_ESP32_COLOR OK
#define ESP32_TSC_9488 OK
//#define ESP32_ILI9341 ERR don't use

//SHIELD
//#define MP_AUDIO OK
//#define MP_A9G OK

//******************************************************************************

/*
update  :   2020/9/8
name    :   MakePython ESP32 SSD1306
sigillum:   MP_ESP32_COLOR
wiki    :   https://www.makerfabs.com/wiki/index.php?title=MakePython_ESP32
module  :   
            (1) 1.3inch OLED SSD1306

*/
#ifdef MP_ESP32

#define MP_ESP32_FLASH_BUTTON 0

//I2C
#define MP_ESP32_I2C_SDA 4
#define MP_ESP32_I2C_SCL 5

//HSPI
#define MP_ESP32_HSPI_MOSI 13
#define MP_ESP32_HSPI_MISO 12
#define MP_ESP32_HSPI_SCK 14
#define MP_ESP32_HSPI_CS 15

//VSPI
#define MP_ESP32_VSPI_MOSI 23
#define MP_ESP32_VSPI_MISO 19
#define MP_ESP32_VSPI_SCK 18
#define MP_ESP32_VSPI_CS 5

//SSD1306
#define MP_ESP32_SSD1306_I2C_ADDR 0x3C
#define MP_ESP32_SSD1306_WIDTH 128 // OLED display width, in pixels
#define MP_ESP32_SSD1306_HEIGHT 64 // OLED display height, in pixels
#define MP_ESP32_SSD1306_RST -1

#endif

/*
update  :   2020/9/11
name    :   MakePython ESP32 COLOR LCD
sigillum:   MP_ESP32_COLOR
wiki    :   https://www.makerfabs.com/wiki/index.php?title=MakePython_ESP32_Color_LCD
module  :   
            (1) 1.3inch TFT ST7789 driver

*/
#ifdef MP_ESP32_COLOR

//I2C
#define MP_ESP32_COLOR_I2C_SDA 4
#define MP_ESP32_COLOR_I2C_SCL 5

//HSPI
#define MP_ESP32_COLOR_HSPI_MOSI 13
#define MP_ESP32_COLOR_HSPI_MISO 12
#define MP_ESP32_COLOR_HSPI_SCK 14
#define MP_ESP32_COLOR_HSPI_CS 15

//VSPI
#define MP_ESP32_COLOR_VSPI_MOSI 23
#define MP_ESP32_COLOR_VSPI_MISO 19
#define MP_ESP32_COLOR_VSPI_SCK 18
#define MP_ESP32_COLOR_VSPI_CS 5

//ST7789
#define MP_ESP32_COLOR_LCD_MOSI MP_ESP32_COLOR_HSPI_MOSI
#define MP_ESP32_COLOR_LCD_MISO MP_ESP32_COLOR_HSPI_MISO
#define MP_ESP32_COLOR_LCD_SCK MP_ESP32_COLOR_HSPI_SCK

#define MP_ESP32_COLOR_LCD_CS 15

#define MP_ESP32_COLOR_LCD_RST 21
#define MP_ESP32_COLOR_LCD_DC 22
#define MP_ESP32_COLOR_LCD_BL -1 //5

#define MP_ESP32_COLOR_LCD_WIDTH 240
#define MP_ESP32_COLOR_LCD_HEIGHT 240
#define MP_ESP32_COLOR_LCD_SPI_HOST HSPI_HOST

#endif

/*
update  :   2020/9/22
name    :   ESP32 Touch Screen Camera With ILI9488
sigillum:   ESP32_TSC_9488 
wiki    :   
module  :   
            (1) 3.5inch TFT ILI9488 driver
            (2) SDcard Reader
            (3) I2C Touch Screen(NS2009 or Ft6236)
            (4) OV2640 Camera
note    :
            (1) In camera need VSPI, but actually use HSPI. Why?
*/

#ifdef ESP32_TSC_9488

//I2C
#define ESP32_TSC_9488_I2C_SDA 26
#define ESP32_TSC_9488_I2C_SCL 27

//SPI
#define ESP32_TSC_9488_HSPI_MOSI 13
#define ESP32_TSC_9488_HSPI_MISO 12
#define ESP32_TSC_9488_HSPI_SCK 14
#define ESP32_TSC_9488_HSPI_CS 15

//ILI9488
#define ESP32_TSC_9488_LCD_MOSI ESP32_TSC_9488_HSPI_MOSI
#define ESP32_TSC_9488_LCD_MISO ESP32_TSC_9488_HSPI_MISO
#define ESP32_TSC_9488_LCD_SCK ESP32_TSC_9488_HSPI_SCK

#define ESP32_TSC_9488_LCD_CS 15

#define ESP32_TSC_9488_LCD_RST 26
#define ESP32_TSC_9488_LCD_DC 33
#define ESP32_TSC_9488_LCD_BL -1

#define ESP32_TSC_9488_LCD_WIDTH 320
#define ESP32_TSC_9488_LCD_HEIGHT 480
#define ESP32_TSC_9488_LCD_SPI_HOST VSPI_HOST   //?? IF use HSPI, will wrong.

//SDcard
#define ESP32_TSC_9488_SD_MOSI ESP32_TSC_9488_HSPI_MOSI
#define ESP32_TSC_9488_SD_MISO ESP32_TSC_9488_HSPI_MISO
#define ESP32_TSC_9488_SD_SCK ESP32_TSC_9488_HSPI_SCK

#define ESP32_TSC_9488_SD_CS 4

#endif

/*
    name    :   ESP32 Touch Screen Camera With ILI9341
    Not ready
*/

#ifdef ESP32_ILI9341

#define LCD_MOSI 13
#define LCD_MISO 12
#define LCD_SCK 14
#define LCD_CS 18
#define LCD_RST 21
#define LCD_DC 22
#define LCD_BL 19 //5

#define LCD_WIDTH 240
#define LCD_HEIGHT 240
#define LCD_SPI_HOST VSPI_HOST

#endif

/*
update  :   2020/9/11
name    :   MakePython ESP32 Audio
sigillum:   MP_AUDIO
wiki    :   https://www.makerfabs.com/wiki/index.php?title=MakePython_Audio
module  :   
            (1) audio jack(3.5mm)
            (2) SD Card
            (3) NXP Low power Audio DAC: uDA1334

*/

#ifdef MP_AUDIO

//SD Card
#define MP_AUDIO_SD_CS 22

#if defined MP_ESP32
    #define MP_AUDIO_SPI_MOSI MP_ESP32_VSPI_MOSI
    #define MP_AUDIO_SPI_MISO MP_ESP32_VSPI_MISO
    #define MP_AUDIO_SPI_SCK MP_ESP32_VSPI_SCK

#elif defined MP_ESP32_COLOR
    #define MP_AUDIO_SPI_MOSI MP_ESP32_COLOR_VSPI_MOSI
    #define MP_AUDIO_SPI_MISO MP_ESP32_COLOR_VSPI_MISO
    #define MP_AUDIO_SPI_SCK MP_ESP32_COLOR_VSPI_SCK

#else
    #define MP_AUDIO_SPI_MOSI 23
    #define MP_AUDIO_SPI_MISO 19
    #define MP_AUDIO_SPI_SCK 18

#endif

//Digital I/O used  //Makerfabs Audio V2.0
#define MP_AUDIO_I2S_DOUT 27
#define MP_AUDIO_I2S_BCLK 26
#define MP_AUDIO_I2S_LRC 25

//Button
#define MP_AUDIO_Pin_vol_up 39
#define MP_AUDIO_Pin_vol_down 36
#define MP_AUDIO_Pin_mute 35
#define MP_AUDIO_Pin_previous 15
#define MP_AUDIO_Pin_pause 33
#define MP_AUDIO_Pin_next 2

#endif


/*
update  :   2020/9/15
name    :   MakePython A9G
sigillum:   MP_A9G
wiki    :   https://www.makerfabs.com/wiki/index.php?title=MakePython_A9G
module  :   
            (1) A9G
            (2) SD Card
            (3) SIM Card

*/

#ifdef MP_A9G

//SD Card
#define MP_A9G_SD_CS 32

#if defined MP_ESP32
    #define MP_A9G_SPI_MOSI MP_ESP32_HSPI_MOSI
    #define MP_A9G_SPI_MISO MP_ESP32_HSPI_MISO
    #define MP_A9G_SPI_SCK MP_ESP32_HSPI_SCK

#elif defined MP_ESP32_COLOR
    #define MP_A9G_SPI_MOSI MP_ESP32_COLOR_HSPI_MOSI
    #define MP_A9G_SPI_MISO MP_ESP32_COLOR_HSPI_MISO
    #define MP_A9G_SPI_SCK MP_ESP32_COLOR_HSPI_SCK

#else
    #define MP_A9G_SPI_MOSI 13
    #define MP_A9G_SPI_MISO 12
    #define MP_A9G_SPI_SCK 14

#endif


#endif