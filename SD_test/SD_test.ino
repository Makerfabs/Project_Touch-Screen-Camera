#include "SPI.h"
#include "SD.h"
#include "FS.h"

//SPI
#define SPI_MOSI 13
#define SPI_MISO 12
#define SPI_SCK 14

#define LCD_CS 15

//SD Card
#define SD_CS 4

//SPI control

#define SPI_ON_SD digitalWrite(SD_CS, LOW)
#define SPI_OFF_SD digitalWrite(SD_CS, HIGH)

String file_list[1000];
int file_num = 0;

void setup()
{
    Serial.begin(115200);

    //CLOSE LCD SPI
    digitalWrite(LCD_CS, HIGH);

    //SPI init
    pinMode(SD_CS, OUTPUT);

    SPI_OFF_SD;

    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

    SPI_ON_SD;
    //SD(SPI) init
    if (!SD.begin(SD_CS, SPI, 800000))
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
}

void loop(void)
{
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

int get_file_list(fs::FS &fs, const char *dirname, uint8_t levels, String filelist[1000])
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
        if(i > 20)
        {
            i--;
            break;
        }
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
