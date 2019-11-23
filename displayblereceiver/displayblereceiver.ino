/* for esp32 */

/* --- neopixel display for esp32 --- */
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

const uint16_t colors[] = { matrix->Color(255, 0, 0), matrix->Color(0, 255, 0), matrix->Color(0, 0, 255) };

int matrixPosX;

/* --- ble --- */

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLEAdvertisedDevice.h>

#define statusLedPin 2

#define DEVICE_NAME "WWBA"

BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;
BLECharacteristic *pRxCharacteristic;
BLEAdvertising *pAdvertising;

//BLEDescriptor *pDescriptor;
bool deviceConnected = false;
bool deviceNotifying = true;
uint8_t messageId = 0;

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

/* --- general vars --- */

unsigned long currentMillis;
unsigned long previousMillis = 0;
int displayInterval = 100;

const int messageQueueLength = 20;
String messages[messageQueueLength];
int newMessageQueueId = 0;
const int defaultMessageQueueLength = 4;
String defaultMessages[defaultMessageQueueLength] = {
    "Dit is de warmste makerspace",
    "Schrijf een bericht voor de makers",
    "Team Scheire maakt oplossingen voor problemen",
    "Volg @canvastv op instagram"
};

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      digitalWrite(statusLedPin, HIGH);
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      digitalWrite(statusLedPin, LOW);
    }
};

class MyCharacteristicCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        messages[newMessageQueueId] = rxValue.c_str();
        newMessageQueueId++;
      }
    }
};

void fetchNextMessage() {
  if (messages[0].length() > 0) {
    matrixText = messages[0];
    for (uint8_t x = 1; x < messageQueueLength; x++) {
      messages[x-1] = messages[x];
    }
    newMessageQueueId--;
  } else {
    int x = random(0, defaultMessageQueueLength);
    matrixText = defaultMessages[x];
  } 
  
  maxDisplacement = matrixText.length() * pixelPerChar + MATRIXWIDTH - 20;
  matrixPosX = MATRIXWIDTH;
  matrix->setTextColor(getMatrixColor());
}

uint16_t getMatrixColor() {
  if (!deviceConnected) {
    return colors[0];
  } else if (messages[0].length() > 0) {
    return colors[2];
  } else {
    return colors[1];
  }
}

bool initBLE() {
  BLEDevice::init(DEVICE_NAME);

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pTxCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX, 
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  pTxCharacteristic->addDescriptor(new BLE2902());

  pRxCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pRxCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pAdvertising = pServer->getAdvertising();
  pAdvertising->addServiceUUID(BLEUUID(SERVICE_UUID));
  BLEAdvertisementData oScanResponseData = BLEAdvertisementData();
  pAdvertising->setScanResponseData(oScanResponseData);
  pAdvertising->start();

  return true;
}

void setup() {
  pinMode(statusLedPin, OUTPUT);
  randomSeed(analogRead(A0));
    
  FastLED.addLeds<NEOPIXEL,PIN>(matrixleds, NUMMATRIX); 
  matrix->begin();
  matrix->setTextWrap(false);
  matrix->setBrightness(40);
  matrix->setCursor(0, 0);
  matrix->setTextColor(getMatrixColor());
  matrix->print("booting...");
  matrix->show();

  if (!initBLE()) {
    matrix->setCursor(0, 0);
    matrix->print("BLE failed");
    matrix->show();
  }

  fetchNextMessage();
}

void loop() {
  currentMillis = millis();
  
  if (currentMillis - previousMillis >= displayInterval) {
    previousMillis = currentMillis;
    matrix->fillScreen(0);
    matrix->setCursor(matrixPosX, 0);
    matrix->print(matrixText);
    if (--matrixPosX < -maxDisplacement) {
      fetchNextMessage();
    }
    matrix->show();
  }
}
