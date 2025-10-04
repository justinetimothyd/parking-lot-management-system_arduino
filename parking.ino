#include <MFRC522.h>
#include <Servo.h>
#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define SS_PIN 10
#define RST_PIN 9

int slot = 3;
int ir1 = 7; 
int ir2 = 6;

MFRC522 mfrc522(SS_PIN, RST_PIN);
Servo myServo;
LiquidCrystal_I2C lcd(0x27, 16, 2); //LCD SIT AP

// Pre-registered UIDs with their corresponding names
const int NUM_USERS = 2;
const String registeredUIDs[NUM_USERS] = {
  "82EFC151", // UID 1
  "D0439458"  // UID 2
};
const String userNames[NUM_USERS] = {
  "Guest 1",   // Name of UID1
  "Guest 2"    // Name of UID2
};

void setup() {
  Serial.begin(9600);//sets the data rate for the serial monitor
  SPI.begin(); //SPI bus(setting MOSI,SCK, SS to outputs); RFID
  mfrc522.PCD_Init(); //initializes the mfrc522; RFID
  myServo.attach(8); //servo pin; servomotor
  myServo.write(0); //default servo degree as per setup; servomotor
  Serial.println("Put your card to the reader..."); //Serial monitor message; debug

  pinMode(ir1, INPUT);
  pinMode(ir2, INPUT);
}

void loop() {
  lcd.begin(); //LCD ISTART
  lcd.backlight();
  lcd.clear();
  lcd.print("Parking Slots: ");
  lcd.print(slot);

  if(slot < 3) { //exit IR
    if(digitalRead(ir2) == LOW) {
      lcd.clear();
      lcd.print("  Drive Safe!");
      slot += 1;
      delay(2000);
    } 
  }
  if(slot <= 0){
    lcd.clear();
    lcd.print("Parking Full");
    delay(1000);
    return;
  }
  //chicks for card
  if(!mfrc522.PICC_IsNewCardPresent() && digitalRead(ir1) == LOW) { 
    lcd.clear();
    lcd.print("  RFID Needed");
    return;
  }
  if(!mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  //Show UID on serial monitor; debugging
  Serial.print("UID TAG: ");
  String content = "";
  for(byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.print("Message: ");
  Serial.println(content);
  content.toUpperCase();

  // Check if the UID matches any registered UID; 
  int userIndex = -1;
  for (int i = 0; i < NUM_USERS; i++) { 
    if (content == registeredUIDs[i]) {
      userIndex = i;
      break;
    }
  }

  if (userIndex != -1 && digitalRead(ir1) == LOW) { 
    if (slot > 0) {
      slot -= 1;
      lcd.clear();
      lcd.print("Welcome, ");
      lcd.setCursor(0, 1);
      lcd.print(userNames[userIndex]);
      Serial.println("Authorized Access"); 
      myServo.write(90);

      while (digitalRead(ir1) == LOW) {
        delay(100);
      }
    delay(2500);
    myServo.write(0); 
    }
  }
  else {
    lcd.clear();
    lcd.print("Access Denied");
    Serial.println("Access Denied");
  }
  delay(1000);
}