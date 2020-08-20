#define ARRAY_LENGTH 320 * 240 * 3

//uint8_t PROGMEM rgb[ARRAY_LENGTH];
//uint8_t rgb[ARRAY_LENGTH];

void setup()
{
    Serial.begin(115200);
    Serial.println("big array test");
    Serial.println(ESP.getFreeHeap());
    Serial.print("befor alloc:");
    Serial.println(heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
}

void loop()
{
    void *ptrVal = NULL;
    ptrVal = heap_caps_malloc(ARRAY_LENGTH, MALLOC_CAP_SPIRAM);
    uint8_t *rgb = (uint8_t *)ptrVal;
    for (int i = 0; i < ARRAY_LENGTH; i++)
    {
        rgb[i] = 0xf0;
    }

    Serial.print("after alloc:");
    Serial.println(heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

    heap_caps_free(ptrVal);
    rgb = NULL;

    Serial.print("after release:");
    Serial.println(heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
}

/*
 void* ptrVal = NULL;
  ptrVal = heap_caps_malloc(1000, MALLOC_CAP_SPIRAM);
  char* sz_ptr = (char*)ptrVal;
  sprintf(sz_ptr, "%s", "hello spi ram..................\n");
 
  Serial.print(sz_ptr);
  
  Serial.print("after alloc:");
  Serial.println(heap_caps_get_free_size( MALLOC_CAP_SPIRAM ));
 
  heap_caps_free(ptrVal);
  sz_ptr = NULL;

    */