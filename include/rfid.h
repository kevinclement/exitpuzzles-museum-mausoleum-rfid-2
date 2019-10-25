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
    bool compareTags();
    void checkForTag_1();
    bool compareIDs(byte idOne[], byte idTwo[]);
    void checkForPuzzleSolved();
    String prettyState(uint8_t);
};