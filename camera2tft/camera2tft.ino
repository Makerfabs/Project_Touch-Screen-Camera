const unsigned short viewBitmap[10] = {
    0xBD, 0x92, 0xD6, 0x55, 0xCE, 0x75, 0xCE, 0x75, 0xCE, 0x55};

void setup()
{
    Serial.begin(115200);
}

void loop()
{
                uint16_t temp = 0;
                for(int i = 0;i < 10;i=i+2)
            temp = viewBitmap[j * w * 2 + 2 * i] <<8 + bitmap[j * w * 2 + 2 * i + 1];
}