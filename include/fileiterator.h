#pragma once
#ifndef _FILEITERATOR_H_
#define _FILEITERATOR_H_

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>

#define SD_CHIP_SELECT 4

namespace file_iterator {
    String getCurrentFilename();
    String getPreviousFilename();
    String getNextFilename();

    File getCurrentFile();
    File findFile(String &filename);

    void setupSD();
    void destroy();
    void moveNext();
    void movePrevious();
    void reset();
};

#endif