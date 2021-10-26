#include <WiFi.h>
#include <WiFiAP.h>
#include "SPI.h"
#include "TFT_eSPI.h"

TFT_eSPI tft = TFT_eSPI();
WiFiServer server(80);
int client_count = 0;
int button_flag = 0;

void setup()
{
    Serial.begin(115200);
    tft.init();
    tft.fillScreen(TFT_BLACK);

    ap_init();
}

void loop()
{
    wifi_server();
}

void ap_init()
{
    //WiFi.softAP(ssid, password);
    WiFi.softAP("Makerfabs_ap");
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);

    tft.setTextColor(TFT_WHITE);

    tft.setCursor(10, 40);
    tft.setTextSize(2);
    tft.println("Makerfabs Web Button");

    tft.setTextSize(1);
    tft.setCursor(10, 60);
    tft.println("Please connect Makerfabs_AP");

    tft.setCursor(10, 70);
    tft.println("And visit the following address");

    tft.setCursor(10, 90);
    tft.setTextSize(1);
    tft.println(myIP);

    draw_button(button_flag);

    server.begin();
}

int wifi_server()
{

    WiFiClient client = server.available(); // listen for incoming clients

    if (client) // if you get a client,
    {
        Serial.println("---------------------------------------------------");
        Serial.printf("Index:%d\n", client_count);
        client_count++;
        Serial.println("New Client."); // print a message out the serial port
        String currentLine = "";       // make a String to hold incoming data from the client
        while (client.connected())
        { // loop while the client's connected
            if (client.available())
            {                           // if there's bytes to read from the client,
                char c = client.read(); // read a byte, then
                Serial.write(c);        // print it out the serial monitor
                if (c == '\n')
                { // if the byte is a newline character

                    // if the current line is blank, you got two newline characters in a row.
                    // that's the end of the client HTTP request, so send a response:
                    if (currentLine.length() == 0)
                    {
                        // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                        // and a content-type so the client knows what's coming, then a blank line:
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println();

                        // the content of the HTTP response follows the header:
                        client.print("<h1>Makerfabs</h1><br><h1>Web Button</h1><br>");

                        client.print("Click <a href=\"/on\">ON</a>.<br>");
                        client.print("Click <a href=\"/off\">OFF</a>.<br>");

                        draw_button(button_flag);

                        // The HTTP response ends with another blank line:
                        client.println();
                        // break out of the while loop:
                        break;
                    }
                    else
                    { // if you got a newline, then clear currentLine:
                        currentLine = "";
                    }
                }
                else if (c != '\r')
                {                     // if you got anything else but a carriage return character,
                    currentLine += c; // add it to the end of the currentLine
                }

                if (currentLine.endsWith("GET /on"))
                {
                    // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                    // and a content-type so the client knows what's coming, then a blank line:
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-type:text/html");
                    client.println();

                    client.print("<h1>Makerfabs</h1><br><h2>Web Button</h2><br>");
                    client.print("<h1>Now : ON</h1><br>");
                    client.print("Click <a href=\"/on\">ON</a>.<br>");
                    client.print("Click <a href=\"/off\">OFF</a>.<br>");

                    // The HTTP response ends with another blank line:
                    client.println();

                    button_flag = 1;
                    draw_button(button_flag);
                    // break out of the while loop:
                    break;
                }

                if (currentLine.endsWith("GET /off"))
                {
                    // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                    // and a content-type so the client knows what's coming, then a blank line:
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-type:text/html");
                    client.println();

                    client.print("<h1>Makerfabs</h1><br><h2>Web Button</h2><br>");
                    client.print("<h1>Now : OFF</h1><br>");
                    client.print("Click <a href=\"/on\">ON</a>.<br>");
                    client.print("Click <a href=\"/off\">OFF</a>.<br>");

                    // The HTTP response ends with another blank line:
                    client.println();

                    button_flag = 0;
                    draw_button(button_flag);
                    // break out of the while loop:
                    break;
                }
            }
        }
        // close the connection:
        client.stop();
        Serial.println("Client Disconnected.");
    }
    return 1;
}

void draw_button(int flag)
{

    if (flag)
    {
        Serial.println("SCREEN ON");
        tft.fillRect(40, 160, 250, 60, TFT_WHITE);
        tft.fillRect(50, 170, 110, 40, TFT_RED);

        tft.setCursor(210, 180);
        tft.setTextColor(TFT_RED);
        tft.setTextSize(2);
        tft.println("ON");
    }
    else
    {
        Serial.println("SCREEN OFF");
        tft.fillRect(40, 160, 250, 60, TFT_WHITE);
        tft.fillRect(170, 170, 110, 40, TFT_BLACK);

        tft.setCursor(90, 180);
        tft.setTextSize(2);
        tft.setTextColor(TFT_BLACK);
        tft.println("OFF");
    }
}