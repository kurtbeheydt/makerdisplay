#include <Adafruit_GFX.h>
#include <FastLED.h>
#include <FastLED_NeoMatrix.h>

#define PIN 13

#define MATRIXWIDTH 64
#define MATRIXHEIGHT 8
#define NUMMATRIX (MATRIXWIDTH * MATRIXHEIGHT)

const int  pixelPerChar = 6;
int  maxDisplacement;
String matrixText;

CRGB matrixleds[NUMMATRIX];

FastLED_NeoMatrix *matrix = new FastLED_NeoMatrix(matrixleds, MATRIXWIDTH, MATRIXHEIGHT, 1, 1, 
  NEO_MATRIX_TOP     + NEO_MATRIX_LEFT +
    NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG );

const uint16_t colors[] = {
  matrix->Color(255, 0, 0), matrix->Color(0, 255, 0), matrix->Color(0, 0, 255) };

int x;
int pass;
unsigned long currentMillis;
unsigned long previousMillis = 0;
int displayInterval = 100;

void setup() {
  FastLED.addLeds<NEOPIXEL,PIN>(matrixleds, NUMMATRIX); 
  matrix->begin();
  matrix->setTextWrap(false);
  matrix->setBrightness(40);
  matrix->setTextColor(colors[0]);
  matrixText = "Welkom in de warmste makerspace";
  x = MATRIXWIDTH;
  pass = 0;
}

void loop() {
  currentMillis = millis();
  
  if (currentMillis - previousMillis >= displayInterval) {
    previousMillis = currentMillis;
    matrix->fillScreen(0);
    matrix->setCursor(x, 0);
    matrix->print(matrixText);
    maxDisplacement = matrixText.length() * pixelPerChar + MATRIXWIDTH;
    if (--x < -maxDisplacement) {
      x = MATRIXWIDTH;
      if (++pass >= 3) pass = 0;
      matrix->setTextColor(colors[pass]);
    }
    matrix->show();
  }

}
