#pragma once

#include "Arduino.h"
#include <MFRC522.h>

enum RFID_STATE {INCORRECT, CORRECT, MISSING, UNKNOWN};

class RfidReader {
  public:
    RfidReader();

    void setup(MFRC522 *mfr, uint8_t pin, byte t[4]);
    void handle();
    RFID_STATE state = UNKNOWN;

  private:
    bool compareTags();
    void printID(byte id[]);
    uint8_t legacyGetID(uint8_t reader);

    MFRC522 *mfr;
    byte tag[4];
    byte readCard[4];

    bool rfid_tag_present_prev = false;
    bool rfid_tag_present = false;
    int _rfid_error_counter = 0;
    bool _tag_found = false;
};