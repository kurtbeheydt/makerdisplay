/*
 * connections:
 * 
 * gpio21 -> SDA on lcd
 * gpio22 -> SCL on lcd
 * 
 * gpio5 -> DataPin
 * gpio18 -> IRQPin
 */

/* --- ble --- */
#include <BLEDevice.h>

static BLEUUID serviceUUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E"); 
static BLEUUID    charUUID("6E400002-B5A3-F393-E0A9-E50E24DCCA9E");

BLEScan* pBLEScan;
static BLEAddress *pServerAddress;
static boolean doConnect = false;
static boolean clientConnected = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;

int scanTime = 10; //In seconds

#define statusLedPin 2
#define DEVICE_NAME "WWKB"

/* --- ps2 keyboard setup --- */
#include <PS2Keyboard.h>
const int DataPin = 5;
const int IRQpin =  18;

PS2Keyboard keyboard;

/* --- lcd feedback screen --- */
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

/* --- general vars --- */
String bericht;

enum State {STATE_TYPING, STATE_CONFIRM };
State state = STATE_TYPING;


/* functions */
static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
  Serial.print("Notify callback for characteristic ");
  for (int i = 0; i < length; i++) {
    Serial.print(pData[i]);
    Serial.print(" ");
  }
  Serial.println();
}

class MyClientCallbacks: public BLEClientCallbacks {
    void onConnect(BLEClient* pClient) {
      clientConnected = true;
      digitalWrite(statusLedPin, HIGH);
      Serial.println("connected");
    };

    void onDisconnect(BLEClient* pClient) {
      clientConnected = false;
      digitalWrite(statusLedPin, LOW);
      Serial.println("disconnected");
    }
};

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());
    Serial.println(My_BLE_Address.c_str());
    
    if (advertisedDevice.getAddress().toString() == "24:0a:c4:30:db:a6") {
      Serial.print("Found our device!  address: ");
      Serial.println(advertisedDevice.getAddress().toString().c_str());
      advertisedDevice.getScan()->stop();
      
      pServerAddress = new BLEAddress(advertisedDevice.getAddress());
      doConnect = true;
    }
  }
};

bool connectToServer(BLEAddress pAddress) {
  Serial.print("Forming a connection to ");
  Serial.println(pAddress.toString().c_str());

  BLEClient* pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(new MyClientCallbacks());
  
  Serial.println(" - Created client");

  // Connect to the remove BLE Server.
  pClient->connect(pAddress);
  Serial.println(" - Connected to server");

  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    return false;
  }
  Serial.println(" - Found our service");

  // Obtain a reference to the characteristic in the service of the remote BLE server.
  pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
  if (pRemoteCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(charUUID.toString().c_str());
    return false;
  }
  Serial.println(" - Found our characteristic");
  pRemoteCharacteristic->writeValue("Connected");
}

void sendText(char* message) {
  if (clientConnected) {
    Serial.printf("*** Sent Value: %s ***\n", message);
    pRemoteCharacteristic->writeValue(message);
  } else {
    Serial.print("*** Value to be sent, but not connected: ");
    Serial.print(message);
    Serial.println(" ***");
  }
}

void setTwoLineText(String firstLine, String secondLine) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(firstLine);
  lcd.setCursor(0, 1);
  lcd.print(secondLine);  
}

void setBerichtOnDisplay() {
  Serial.println(bericht);
  if (bericht.length() < 1) {
    setTwoLineText("typ je bericht", "voor de makers");
  } else {
    setTwoLineText(bericht, bericht.substring(16));
  }
}

void setDefaultText() {
  bericht = "";
  setTwoLineText("typ je bericht", "voor de makers");
}

char* string2char(String command){
  if (command.length() != 0) {
    char *p = const_cast<char*>(command.c_str());
    return p;
  }
}

void setup() {
  pinMode(statusLedPin, OUTPUT);
    
  keyboard.begin(DataPin, IRQpin, PS2Keymap_BE);

  lcd.begin();
  lcd.backlight();
  lcd.clear();

  setDefaultText();

  Serial.begin(115200);
  Serial.println("Keyboard input ready");

  BLEDevice::init(DEVICE_NAME);
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);

  Serial.println("pedalBoard started ...");

  delay(300);
}

void loop() {
  if (doConnect == true) {
    setTwoLineText("Connecting to", "display");

    if (connectToServer(*pServerAddress)) {
      Serial.println("SUCCESS: connected to display.");
      setTwoLineText("Display", "connected");
      delay(1000);
      setDefaultText();
    } else {
      setTwoLineText("Connection", "failed");
      Serial.println("ERROR: failed to connect to display.");
    }
    doConnect = false;
  } else {
    if ((!doConnect) && (clientConnected)) {
      if (keyboard.available()) {
      
        // read the next key
        char c = keyboard.read();
    
        if (state == STATE_CONFIRM) {
          if (c == PS2_ENTER) {
            if (bericht.length() > 0) {
              Serial.println("Sending Message");
              setTwoLineText("bericht wordt", "verstuurd.");
              sendText(string2char(bericht));
              delay(2000);
              setTwoLineText("bericht is", "verstuurd.");
              delay(2000);
            }
            state = STATE_TYPING;
            setDefaultText();
          } else {
            state = STATE_TYPING;
          }
        }
        
        if (state == STATE_TYPING) {
          if (c == PS2_ENTER) {
            Serial.println("[enter]");
            if (bericht.length() > 0) {
              state = STATE_CONFIRM;
              setTwoLineText("   versturen?   ", " Ja: druk enter ");
            } else {
              setDefaultText();
            }
          } else if (c == PS2_BACKSPACE) {
            Serial.println("[Backspace]");
            bericht = bericht.substring(0, bericht.length() - 1);
            setBerichtOnDisplay();
          } else if (c == PS2_ESC) {
            Serial.println("[ESC]");
            //setDefaultText();
          } else if (c == PS2_DELETE) {
            Serial.println("[Del]");
            setDefaultText();
          } else {
            Serial.println(c);
            bericht = bericht + c;
            setBerichtOnDisplay();
          }
        }
      }
    } else {
      Serial.println("Scanning...");
      setTwoLineText("Searching for", "Display");
      pBLEScan->start(scanTime);
      Serial.println("Scan done!");
    }
  }
}
