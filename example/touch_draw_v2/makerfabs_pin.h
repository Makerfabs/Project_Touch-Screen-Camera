//#define ESP32_COLOR_7789 OK
#define ESP32_ILI9488 OK
//#define ESP32_ILI9341 ERR don't use

#ifdef ESP32_COLOR_7789

#define LCD_MOSI    13
#define LCD_MISO    12
#define LCD_SCK     14
#define LCD_CS      15
#define LCD_RST     21
#define LCD_DC      22
#define LCD_BL      -1  //5

#define LCD_WIDTH   240
#define LCD_HEIGHT  240
#define LCD_SPI_HOST VSPI_HOST

#endif

#ifdef ESP32_ILI9488

#define I2C_SDA 26
#define I2C_SCL 27

#define SPI_MOSI 13
#define SPI_MISO 12
#define SPI_SCK 14

#define LCD_MOSI    SPI_MOSI
#define LCD_MISO    SPI_MISO
#define LCD_SCK     SPI_SCK
#define LCD_CS      15
#define LCD_RST     26
#define LCD_DC      33
#define LCD_BL      -1  

#define LCD_WIDTH   320
#define LCD_HEIGHT  480
#define LCD_SPI_HOST HSPI_HOST

#endif

#ifdef ESP32_ILI9341

#define LCD_MOSI    13
#define LCD_MISO    12
#define LCD_SCK     14
#define LCD_CS      18
#define LCD_RST     21
#define LCD_DC      22
#define LCD_BL      19  //5

#define LCD_WIDTH   240
#define LCD_HEIGHT  240
#define LCD_SPI_HOST VSPI_HOST

#endif