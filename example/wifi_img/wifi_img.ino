
#include <SPI.h>
#include <mySD.h>
#include <Adafruit_GFX.h>
#include "ILI9488.h"

#include <WiFiClient.h>
#include <ESP32WebServer.h>
#include <WiFi.h>
#include <ESPmDNS.h>

//SPI
#define SPI_MOSI 13
#define SPI_MISO 12
#define SPI_SCK 14

//SD
#define SD_CS 4

//TFT
#define TFT_CS 15
#define TFT_DC 33
#define TFT_LED -1 //1//-1
#define TFT_RST -1 //3//-1

const char *ssid = "Makerfabs";
const char *password = "20160704";

#define PIC_NAME "/display.bmp"

ILI9488 tft = ILI9488(TFT_CS, TFT_DC, TFT_RST);
ESP32WebServer server(80);
File root;
bool opened = false;

void setup(void)
{
    Serial.begin(115200);

    //SPI init
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

    tft.begin();
    tft.setRotation(3);

    tft.fillScreen(ILI9488_BLUE);

    tft.setCursor(10, 10);
    tft.setTextColor(ILI9488_WHITE);
    tft.setTextSize(4);
    tft.println("WIFI FILESYSTEM");

    if (!SD.begin(SD_CS))
    {
        Serial.println("initialization failed!");
        return;
    }

    tft.setCursor(10, 40);
    tft.setTextColor(ILI9488_WHITE);
    tft.setTextSize(4);
    tft.println("SD INIT OVER");

    webserve_init();

    tft.println(WiFi.localIP());
    //print_img("/logo.bmp");
}

void loop(void)
{
    server.handleClient();
}

//Init

void webserve_init()
{
    WiFi.begin(ssid, password);
    Serial.println("");

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    //use IP or iotsharing.local to access webserver
    if (MDNS.begin("iotsharing"))
    {
        Serial.println("MDNS responder started");
    }

    Serial.println("initialization done.");
    //handle uri
    server.on("/", handleRoot);
    server.onNotFound(handleNotFound);

    /*handling uploading file */
    server.on(
        "/update", HTTP_POST, []()
        { server.sendHeader("Connection", "close"); },
        []()
        {
            HTTPUpload &upload = server.upload();

            String upload_file_name = String("/") + upload.filename;
            if (opened == false)
            {
                opened = true;

                //Fresh File
                SD.remove((char *)upload_file_name.c_str());
                root = SD.open(upload_file_name.c_str(), FILE_WRITE);
                if (!root)
                {
                    Serial.println("- failed to open file for writing");
                    return;
                }
            }
            if (upload.status == UPLOAD_FILE_WRITE)
            {
                if (root.write(upload.buf, upload.currentSize) != upload.currentSize)
                {
                    Serial.println("- failed to write");
                    return;
                }
            }
            else if (upload.status == UPLOAD_FILE_END)
            {
                root.close();
                Serial.println("");
                Serial.println("UPLOAD_FILE_END");
                opened = false;

                //Display Upload File
                if (upload_file_name.endsWith(".bmp"))
                    print_img(upload_file_name);
                else if (upload_file_name.endsWith(".txt"))
                    print_txt(upload_file_name);
            }
        });
    server.begin();
    Serial.println("HTTP server started");
}

//Web Server

String serverIndex = "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
                     "<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
                     "<input type='file' name='update'>"
                     "<input type='submit' value='Upload'>"
                     "</form>"
                     "<div id='prg'>progress: 0%</div>"
                     "<script>"
                     "$('form').submit(function(e){"
                     "e.preventDefault();"
                     "var form = $('#upload_form')[0];"
                     "var data = new FormData(form);"
                     " $.ajax({"
                     "url: '/update',"
                     "type: 'POST',"
                     "data: data,"
                     "contentType: false,"
                     "processData:false,"
                     "xhr: function() {"
                     "var xhr = new window.XMLHttpRequest();"
                     "xhr.upload.addEventListener('progress', function(evt) {"
                     "if (evt.lengthComputable) {"
                     "var per = evt.loaded / evt.total;"
                     "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
                     "}"
                     "}, false);"
                     "return xhr;"
                     "},"
                     "success:function(d, s) {"
                     "console.log('success!')"
                     "},"
                     "error: function (a, b, c) {"
                     "}"
                     "});"
                     "});"
                     "</script>";

String printDirectory(File dir, int numTabs)
{
    String response = "";
    dir.rewindDirectory();

    while (true)
    {
        File entry = dir.openNextFile();
        if (!entry)
        {
            // no more files
            //Serial.println("**nomorefiles**");
            break;
        }
        for (uint8_t i = 0; i < numTabs; i++)
        {
            Serial.print('\t'); // we'll have a nice indentation
        }
        // Recurse for directories, otherwise print the file size
        if (entry.isDirectory())
        {
            printDirectory(entry, numTabs + 1);
        }
        else
        {
            response += String("<a href='") + String(entry.name()) + String("'>") + String(entry.name()) + String("</a>") + String("</br>");
        }
        entry.close();
    }
    return String("List files:</br>") + response + String("</br></br> Upload file:") + serverIndex;
}

void handleRoot()
{
    root = SD.open("/");
    String res = printDirectory(root, 0);
    server.send(200, "text/html", res);
}

bool loadFromSDCARD(String path)
{
    path.toLowerCase();
    String dataType = "text/plain";
    if (path.endsWith("/"))
        path += "index.htm";

    if (path.endsWith(".src"))
        path = path.substring(0, path.lastIndexOf("."));
    else if (path.endsWith(".jpg"))
        dataType = "image/jpeg";
    else if (path.endsWith(".txt"))
        dataType = "text/plain";
    else if (path.endsWith(".zip"))
        dataType = "application/zip";
    Serial.println(dataType);
    File dataFile = SD.open(path.c_str());

    Serial.println(path.c_str());

    if (!dataFile)
        return false;

    if (server.streamFile(dataFile, dataType) != dataFile.size())
    {
        Serial.println("Sent less data than expected!");
    }

    dataFile.close();
    return true;
}

void handleNotFound()
{
    if (loadFromSDCARD(server.uri()))
        return;
    String message = "SDCARD Not Detected\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i = 0; i < server.args(); i++)
    {
        message += " NAME:" + server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
    }
    server.send(404, "text/plain", message);
    Serial.println(message);
}

//Image Display
int print_img(String filename)
{
    File f = SD.open(filename.c_str());
    if (!f)
    {
        Serial.println("Failed to open file for reading");
        return 0;
    }

    int X = 480;
    int Y = 320;
    uint8_t RGB[3 * X];
    for (int row = 0; row < Y; row++)
    {
        f.seek(54 + 3 * X * row);
        f.read(RGB, 3 * X);
        for (int col = 0; col < X; col++)
        {
            tft.drawPixel(col, Y - 1 - row, tft.color565(RGB[col * 3 + 2], RGB[col * 3 + 1], RGB[col * 3]));
        }
    }

    f.close();

    return 0;
}

int print_txt(String filename)
{
    File f = SD.open(filename.c_str());
    if (!f)
    {
        Serial.println("Failed to open file for reading");
        return 0;
    }
    Serial.println("");

    int pos_y = 10;

    tft.fillScreen(ILI9488_BLACK);
    tft.setTextColor(ILI9488_WHITE);
    tft.setTextSize(4);

    tft.setCursor(10, pos_y);

    String oneline = "";

    while (f.available())
    {
        int c = f.read();
        if (c == '\n')
        {
            tft.println(oneline);
            Serial.println(oneline);
            oneline = "";
            tft.setCursor(10, pos_y += 32);
        }
        else
        {
            oneline += (char)c;
        }
    }

    tft.println(oneline);
    Serial.println(oneline);

    f.close();
    Serial.println("");

    return 0;
}
