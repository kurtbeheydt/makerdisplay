/* for esp32 */

/* --- neopixel display --- */

#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 13 //(D13)
#define MATRIX_HEIGHT 8
#define MATRIX_WIDTH 32
const int  pixelPerChar = 6;
int  maxDisplacement;
String matrixText;

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(MATRIX_WIDTH, MATRIX_HEIGHT, 2, 1, MATRIX_PIN,
  NEO_TILE_TOP   + NEO_TILE_LEFT   + NEO_TILE_COLUMNS   + NEO_TILE_PROGRESSIVE +
  NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
  NEO_GRB + NEO_KHZ800);

int x = matrix.width();
int pass = 0;

const uint16_t colors[] = {
  matrix.Color(255, 255, 255), matrix.Color(255, 0, 0), matrix.Color(0, 0, 255) };

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

void setNewDisplayText(String newText) {
  matrix.setTextColor(colors[0]);
  matrixText = newText;
  maxDisplacement = matrixText.length() * pixelPerChar + matrix.width();
  x = matrix.width();
}

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      digitalWrite(statusLedPin, HIGH);
      setNewDisplayText("bluetooth connected");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      digitalWrite(statusLedPin, LOW);
      setNewDisplayText("bluetooth disconnected");
    }
};

class MyCharacteristicCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        setNewDisplayText(rxValue.c_str());
/*        Serial.println("*********");
        Serial.print("Received Value: ");
        for (int i = 0; i < rxValue.length(); i++) {
          Serial.print(rxValue[i]);
        }
        
        Serial.println();
        Serial.println("*********");
*/
      }
    }
};

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
  //pDescriptor->setCallbacks(new MyDescriptorCallbacks());

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
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(20);
  setNewDisplayText("Welkom in de warmste makerspace");

  if (!initBLE()) {
    setNewDisplayText("Bluetooth init failed");
  }
}

void loop() {
  currentMillis = millis();
  
  if (currentMillis - previousMillis >= displayInterval) {
    previousMillis = currentMillis;
    matrix.fillScreen(0);
    matrix.setCursor(x, 0);
    matrix.print(matrixText);
    if (--x < -maxDisplacement) {
      x = matrix.width();
      if (++pass >= 3) pass = 0;
      matrix.setTextColor(colors[pass]);
    }
    matrix.show();
  }
}
