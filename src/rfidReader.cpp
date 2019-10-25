#include "Arduino.h"
#include "rfidReader.h"

RfidReader::RfidReader()
{
}

void RfidReader::setup(MFRC522 *mfr, uint8_t pin, byte t[4]) {

  Serial.print("Reader Pin: ");
  Serial.print(pin);
  Serial.print(" => ");
  
  mfr->PCD_Init();
  delay(4);
  mfr->PCD_DumpVersionToSerial();

  tag[0] = t[0];
  tag[1] = t[1];
  tag[2] = t[2];
  tag[3] = t[3];
}

void RfidReader::handle() {
  rfid_tag_present_prev = rfid_tag_present;

  _rfid_error_counter += 1;
  if(_rfid_error_counter > 2){
    _tag_found = false;
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
      return;
    }
    _rfid_error_counter = 0;
    _tag_found = true;

    for ( uint8_t i = 0; i < 4; i++) {
       readCard[i] = mfr->uid.uidByte[i];
    }
  }

  rfid_tag_present = _tag_found;
  
  // rising edge
  if (rfid_tag_present && !rfid_tag_present_prev){
    Serial.println("Tag found, checking...");
    state = compareTags() ? CORRECT : INCORRECT;
  }

  // falling edge
  if (!rfid_tag_present && rfid_tag_present_prev){
    Serial.println("Tag gone");
    state = MISSING;
  }
}

bool RfidReader::compareTags() {
  for ( uint8_t k = 0; k < 4; k++ ) {
    if ( readCard[k] != tag[k] ) {
       return false;
    }
  }
  return true;
}

void RfidReader::printID(byte id[]) {
  for ( uint8_t i = 0; i < 4; i++) {  //
    Serial.print(id[i] < 0x10 ? "0" : "");
    Serial.print(id[i], HEX);
  }
}

uint8_t RfidReader::legacyGetID(uint8_t reader) {
  // Getting ready for Reading PICCs
  if (!mfr->PICC_IsNewCardPresent()) {
    return 0;
  }
  if (!mfr->PICC_ReadCardSerial()) {
    return 0;
  }

  for ( uint8_t i = 0; i < 4; i++) {  //
    readCard[i] = mfr->uid.uidByte[i];
  }

  // Stop reading
  mfr->PICC_HaltA();
  mfr->PCD_StopCrypto1();

  return 1;
}
