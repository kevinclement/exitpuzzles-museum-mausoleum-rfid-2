#include "Arduino.h"
#include "rfid.h"
#include "logic.h"
#include "rfidReader.h"
#include <MFRC522.h>
#include <SPI.h>

MFRC522 mfrc522_1(8, 7);
MFRC522 mfrc522_2(10, 9);
RfidReader reader[NR_OF_READERS];

byte tags[][4] = {
  { 0xA7, 0x86, 0x8A, 0xF2 },
  { 0xF7, 0x81, 0x8A, 0xF2 }
};

Rfid::Rfid(Logic &logic)
: _logic(logic)
{
}

void Rfid::setup() {
  SPI.begin();

  mfrc522_1.PCD_Init();
  mfrc522_2.PCD_Init();

  delay(4);
  mfrc522_1.PCD_DumpVersionToSerial();
  mfrc522_2.PCD_DumpVersionToSerial();
  
  Serial.println("\nReady to Scan...");
}

void Rfid::handle() {
  checkForTag_1();
  // if ( mfrc522_1.PICC_IsNewCardPresent()) {
  //   if (mfrc522_1.PICC_ReadCardSerial()) {
  //     Serial.println("@@@ CARD 1 @@@");
  //     mfrc522_1.PICC_DumpToSerial(&(mfrc522_1.uid));
  //   }
	// }

  // if ( mfrc522_2.PICC_IsNewCardPresent()) {
  //   Serial.println("NEW CARD");
  //   if (mfrc522_2.PICC_ReadCardSerial()) {
  //     Serial.println("### CARD 2 ###");
  //     mfrc522_2.PICC_DumpToSerial(&(mfrc522_2.uid));
  //   }
	// }

  // for (uint8_t i = 0; i < NR_OF_READERS; i++) {

  //   reader[i].handle();

  //   if (state[i] != reader[i].state) {
  //     Serial.print("state changed for ");
  //     Serial.print(i + 1);
  //     Serial.print(" ");
  //     Serial.print(prettyState(state[i]));
  //     Serial.print(" => ");
  //     Serial.println(prettyState(reader[i].state));

  //     state[i] = reader[i].state;
  //     checkForPuzzleSolved();
  //     _logic.status();
  //   }

  //   //     Serial.print("UNKNOWN tag ");
  //   //     printID(readCard);
  //   //     Serial.print(" on reader ");
  //   //     Serial.print(i);
  // }
}
bool rfid_tag_present_prev = false;
bool rfid_tag_present = false;
int _rfid_error_counter = 0;
bool _tag_found = false;
RFID_STATE state_1 = UNKNOWN;
byte readCard[4];

void Rfid::checkForTag_1() {
  rfid_tag_present_prev = rfid_tag_present;

  _rfid_error_counter += 1;
  if(_rfid_error_counter > 2){
    _tag_found = false;
  }

  // Detect Tag without looking for collisions
  byte bufferATQA[2];
  byte bufferSize = sizeof(bufferATQA);

  // Reset baud rates
  mfrc522_1.PCD_WriteRegister(mfrc522_1.TxModeReg, 0x00);
  mfrc522_1.PCD_WriteRegister(mfrc522_1.RxModeReg, 0x00);

  // Reset ModWidthReg
  mfrc522_1.PCD_WriteRegister(mfrc522_1.ModWidthReg, 0x26);

  MFRC522::StatusCode result = mfrc522_1.PICC_RequestA(bufferATQA, &bufferSize);

  if(result == mfrc522_1.STATUS_OK){
    if ( ! mfrc522_1.PICC_ReadCardSerial()) { //Since a PICC placed get Serial and continue   
      return;
    }
    _rfid_error_counter = 0;
    _tag_found = true;

    for ( uint8_t i = 0; i < 4; i++) {
       readCard[i] = mfrc522_1.uid.uidByte[i];
    }
  }

  rfid_tag_present = _tag_found;
  
  // rising edge
  if (rfid_tag_present && !rfid_tag_present_prev){
    Serial.println("Tag found, checking...");
    state_1 = compareTags() ? CORRECT : INCORRECT;
  }

  // falling edge
  if (!rfid_tag_present && rfid_tag_present_prev){
    Serial.println("Tag gone");
    state_1 = MISSING;
  }
}

bool Rfid::isIdol(byte id[], uint8_t reader) {
  return compareIDs(id, tags[reader]);
}

bool Rfid::compareTags() {
  for ( uint8_t k = 0; k < 4; k++ ) {
    if ( readCard[k] != tags[0][k] ) {
       return false;
    }
  }
  return true;
}

bool Rfid::compareIDs(byte idOne[], byte idTwo[] ) {   
  for ( uint8_t k = 0; k < 4; k++ ) {
    if ( idOne[k] != idTwo[k] ) {
       return false;
    }
  }
  return true;
}

void Rfid::checkForPuzzleSolved() {
  solved = true;
  for (uint8_t i = 0; i < NR_OF_READERS; i++) {
    solved = solved && state[i] == CORRECT;
  }
}

String Rfid::prettyState(uint8_t state) {
  return 
    state == INCORRECT ? "Incorrect" : 
    state == CORRECT ? "Correct" : 
    state == MISSING ? "Missing" : 
    "Unknown";
}