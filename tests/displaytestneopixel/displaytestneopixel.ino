
// Include Libraries
#include "Arduino.h"
#include "Adafruit_NeoPixel.h"


// Pin Definitions
#define LEDMATRIXRGB_PIN_DIN  13



// Global variables and defines
#define LedMatrixRGB_NUM_LEDS 64*1
#define LedMatrixRGB_BRIGHTNESS 10
// object initialization
Adafruit_NeoPixel LedMatrixRGB = Adafruit_NeoPixel(LedMatrixRGB_NUM_LEDS, LEDMATRIXRGB_PIN_DIN, NEO_GRBW + NEO_KHZ800);


// define vars for testing menu
const int timeout = 10000;       //define timeout of 10 sec
char menuOption = 0;
long time0;

// Setup the essentials for your circuit to work. It runs first every time your circuit is powered with electricity.
void setup() 
{
    // Setup Serial which is useful for debugging
    // Use the Serial Monitor to view printed messages
    Serial.begin(9600);
    while (!Serial) ; // wait for serial port to connect. Needed for native USB
    Serial.println("start");
    
    LedMatrixRGB.setBrightness(LedMatrixRGB_BRIGHTNESS);
    LedMatrixRGB.begin();
    LedMatrixRGB.show(); // Initialize all pixels to 'off'
    menuOption = menu();
    
}

// Main logic of your circuit. It defines the interaction between the components you selected. After setup, it runs over and over again, in an eternal loop.
void loop() 
{
    
    
    if(menuOption == '1') {
    // Adafruit NeoPixel NeoMatrix 8x8 - 64 RGB LED Pixel Matrix - Test Code
    for (uint16_t i = 0; i < LedMatrixRGB.numPixels(); i++) {
    LedMatrixRGB.setPixelColor(i, 255, 0, 0);
    LedMatrixRGB.show();
    delay(100);
    }
    for (uint16_t i = 0; i < LedMatrixRGB.numPixels(); i++) {
    LedMatrixRGB.setPixelColor(i, 0, 255, 0);
    LedMatrixRGB.show();
    delay(80);
    }
    for (uint16_t i = 0; i < LedMatrixRGB.numPixels(); i++) {
    LedMatrixRGB.setPixelColor(i, 0, 0, 255);
    LedMatrixRGB.show();
    delay(30);
    }
    }
    
    if (millis() - time0 > timeout)
    {
        menuOption = menu();
    }
    
}



// Menu function for selecting the components to be tested
// Follow serial monitor for instrcutions
char menu()
{

    Serial.println(F("\nWhich component would you like to test?"));
    Serial.println(F("(1) Adafruit NeoPixel NeoMatrix 8x8 - 64 RGB LED Pixel Matrix"));
    Serial.println(F("(menu) send anything else or press on board reset button\n"));
    while (!Serial.available());

    // Read data from serial monitor if received
    while (Serial.available()) 
    {
        char c = Serial.read();
        if (isAlphaNumeric(c)) 
        {   
            
            if(c == '1') 
          Serial.println(F("Now Testing Adafruit NeoPixel NeoMatrix 8x8 - 64 RGB LED Pixel Matrix"));
            else
            {
                Serial.println(F("illegal input!"));
                return 0;
            }
            time0 = millis();
            return c;
        }
    }
}
