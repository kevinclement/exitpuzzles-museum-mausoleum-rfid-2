#include "Arduino.h"
#include "rfid.h"
#include "logic.h"
#include <MFRC522.h>
#include <SPI.h>

MFRC522 mfrc522_1(8, 7);
MFRC522 mfrc522_2(10, 9);

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
  //NR_OF_READERS
  for (uint8_t i = 0; i < 1; i++) {
    RFID_STATE st = checkForTag(0, &mfrc522_1);
    if (st != state[i]) {
      Serial.print("state changed for ");
      Serial.print(i + 1);
      Serial.print(" ");
      Serial.print(prettyState(state[i]));
      Serial.print(" => ");
      Serial.println(prettyState(st));

      state[i] = st;
      checkForPuzzleSolved();
      _logic.status();
    }
  }
}

RFID_STATE Rfid::checkForTag(uint8_t index, MFRC522 *mfr) {
  RFID_STATE st = state[index];
  tag_present_prev[index] = tag_present[index];

  error_counter[index] += 1;
  if(error_counter[index] > 2){
    tag_found[index] = false;
  }

  // Detect Tag without looking for collisions
  byte bufferATQA[2];
  byte bufferSize = sizeof(bufferATQA);

  // Reset baud rates
  mfr->PCD_WriteRegister(mfr->TxModeReg, 0x00);
  mfr->PCD_WriteRegister(mfr->RxModeReg, 0x00);

  // Reset ModWidthReg
  mfr->PCD_WriteRegister(mfr->ModWidthReg, 0x26);

  MFRC522::StatusCode result = mfr->PICC_RequestA(bufferATQA, &bufferSize);

  if(result == mfr->STATUS_OK){
    if ( ! mfr->PICC_ReadCardSerial()) { //Since a PICC placed get Serial and continue   
      return st;
    }
    error_counter[index] = 0;
    tag_found[index] = true;

    for ( uint8_t i = 0; i < 4; i++) {
       readCards[index][i] = mfr->uid.uidByte[i];
    }
  }

  tag_present[index] = tag_found[index];

  // rising edge
  if (tag_present[index] && !tag_present_prev[index]){
    Serial.println("Tag found, checking...");
    st = compareTags(index) ? CORRECT : INCORRECT;
  }

  // falling edge
  if (!tag_present[index] && tag_present_prev[index]){
    Serial.println("Tag gone");
    st = MISSING;
  }

  return st;
}

bool Rfid::isIdol(byte id[], uint8_t reader) {
  return compareIDs(id, tags[reader]);
}

bool Rfid::compareTags(uint8_t index) {
  for ( uint8_t k = 0; k < 4; k++ ) {
    if ( readCards[index][k] != tags[index][k] ) {
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