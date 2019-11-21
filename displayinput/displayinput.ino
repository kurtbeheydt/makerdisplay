#include <PS2Keyboard.h>
const int DataPin = 2;
const int IRQpin =  3;

PS2Keyboard keyboard;

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

String bericht;

enum State {STATE_TYPING, STATE_CONFIRM };
State state = STATE_TYPING;

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

void setup() {
  keyboard.begin(DataPin, IRQpin, PS2Keymap_BE);

  lcd.begin();
  lcd.backlight();
  lcd.clear();

  setDefaultText();

  Serial.begin(115200);
  Serial.println("Keyboard Test:");

  delay(300);
}

void loop() {
  if (keyboard.available()) {
    
    // read the next key
    char c = keyboard.read();

    if (state == STATE_CONFIRM) {
      if (c == PS2_ENTER) {
        Serial.println("Sending Message");
        setTwoLineText("bericht wordt", "verstuurd.");
        delay(2000);
        setTwoLineText("bericht is", "verstuurd.");
        delay(2000);
        state = STATE_TYPING;
        setDefaultText();
      } else {
        state = STATE_TYPING;
      }
    }
    
    if (state == STATE_TYPING) {
      if (c == PS2_ENTER) {
        Serial.println("[enter]");
        state = STATE_CONFIRM;
        setTwoLineText("   versturen?   ", " Ja: druk enter ");
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
}
