#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

#define PIN 6

#define MATRIX_HEIGHT 8
#define MATRIX_WIDTH 32
const int  pixelPerChar = 6;
int  maxDisplacement;
String matrixText;

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(MATRIX_WIDTH, MATRIX_HEIGHT, 2, 1, PIN,
  NEO_TILE_TOP   + NEO_TILE_LEFT   + NEO_TILE_COLUMNS   + NEO_TILE_PROGRESSIVE +
  NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
  NEO_GRB + NEO_KHZ800);

int x = matrix.width();
int pass = 0;

const uint16_t colors[] = {
  matrix.Color(255, 255, 255), matrix.Color(255, 0, 0), matrix.Color(0, 0, 255) };

unsigned long currentMillis;
unsigned long previousMillis = 0;
int displayInterval = 100;

void setup() {
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(60); // TODO via ldr
  matrix.setTextColor(colors[0]);
  matrixText = "Welkom in de warmste makerspace";
}

void loop() {
  currentMillis = millis();
  
  if (currentMillis - previousMillis >= displayInterval) {
    previousMillis = currentMillis;
    matrix.fillScreen(0);
    matrix.setCursor(x, 0);
    matrix.print(matrixText);
    maxDisplacement = matrixText.length() * pixelPerChar + matrix.width();
    if (--x < -maxDisplacement) {
      x = matrix.width();
      if (++pass >= 3) pass = 0;
      matrix.setTextColor(colors[pass]);
    }
    matrix.show();
  }
}
