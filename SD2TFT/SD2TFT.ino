/***************************************************
  This is our GFX example for the Adafruit ILI9488 Breakout and Shield
  ----> http://www.adafruit.com/products/1651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

#include "SPI.h"
#include <Adafruit_GFX.h>
#include "ILI9488.h"
#include "SD.h"
#include "FS.h"

//SPI
#define SPI_MOSI 13
#define SPI_MISO 12
#define SPI_SCK 14

//SD Card
#define SD_CS 4

//TFT
#define TFT_CS 15
#define TFT_DC 33
#define TFT_LED -1 //1//-1
#define TFT_RST -1 //3//-1

//SPI control
#define SPI_ON_TFT digitalWrite(TFT_CS, LOW)
#define SPI_OFF_TFT digitalWrite(TFT_CS, HIGH)
#define SPI_ON_SD digitalWrite(SD_CS, LOW)
#define SPI_OFF_SD digitalWrite(SD_CS, HIGH)

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
ILI9488 tft = ILI9488(TFT_CS, TFT_DC, TFT_RST);
// If using the breakout, change pins as desired
//ILI9488 tft = ILI9488(TFT_CS, TFT_DC, SPI_MOSI, SPI_SCK, TFT_RST, SPI_MISO);

//Makerfabs_ILI9488 tft = Makerfabs_ILI9488(TFT_CS, TFT_DC);

String file_list[20];
int file_num = 0;

void setup()
{
    Serial.begin(115200);

    //SPI init
    pinMode(SD_CS, OUTPUT);
    pinMode(TFT_CS, OUTPUT);
    SPI_OFF_SD;
    SPI_OFF_TFT;
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

    SPI_ON_SD;
    //SD(SPI) init
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

    //TFT(SPI) init
    SPI_ON_TFT;
    tft.begin();
    tft.setRotation(2);
    SPI_OFF_TFT;
}

void loop(void)
{
    print_img(SD, "/cat.bmp");
    //sd_test();

    delay(1000);

    print_img(SD, "/red.bmp");

    delay(1000);
}

void sd_test()
{
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
    digitalWrite(SD_CS, LOW);
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

int read_img_msg(fs::FS &fs, const char *filename)
{
    SPI_ON_SD;
    File f = fs.open(filename);
    if (!f)
    {
        Serial.println("Failed to open file for reading");
        return 0;
    }
    /*
    Serial.print("Read from file: ");
    while (f.available())
    {
        Serial.write(f.read());
    }
    */
    // bmp file header
    // 文件标识符，必须为"BM"，即0x424D 才是Windows位图文件
    f.seek(0);
    char bfType[3];
    f.readBytes(bfType, 2);
    bfType[2] = '\0';

    // 整个BMP文件的大小（以位B为单位）
    f.seek(0x2);
    uint8_t bfSize[4];
    f.read(bfSize, 4);

    // 说明从文件头0000h开始到图像像素数据的字节偏移量（以字节Bytes为单位）
    f.seek(0xa);
    uint8_t bfOffBits[4];
    f.read(bfOffBits, 4);

    Serial.println(bfType);
    //Serial.println(bfSize[0] + bfSize[1] + bfSize[2] + bfSize[3]);
    //Serial.println(bfOffBits);
    f.close();
    SPI_OFF_SD;
    return 0;
}

int print_img(fs::FS &fs, const char *filename)
{
    SPI_ON_SD;
    File f = fs.open(filename);
    if (!f)
    {
        Serial.println("Failed to open file for reading");
        return 0;
    }

    // 从54偏移位置开始读
    f.seek(54);
    #define X 320
    #define Y 240
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
