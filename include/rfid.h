#pragma once

#include "Arduino.h"
#include "rfidReader.h"

class Logic;

#define NR_OF_READERS   2

class Rfid {
  public:
    Rfid(Logic &logic);
    void setup();
    void handle();
    bool solved = false;
    RFID_STATE state[NR_OF_READERS] = { UNKNOWN, UNKNOWN };

  private:
    Logic &_logic;
    uint8_t getID(uint8_t reader);
    bool isIdol(byte id[], uint8_t reader);
    bool compareTags(uint8_t index);
    void checkForTag(uint8_t index, MFRC522 *mfr);
    bool compareIDs(byte idOne[], byte idTwo[]);
    void checkForPuzzleSolved();
    String prettyState(uint8_t);

    bool tag_present_prev[NR_OF_READERS] = { false, false };
    bool tag_present[NR_OF_READERS] = { false, false };
    int error_counter[NR_OF_READERS] = { 0, 0 };
    bool tag_found[NR_OF_READERS] = { false, false };
    byte readCards[NR_OF_READERS][4];
};