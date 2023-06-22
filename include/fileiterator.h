#pragma once
#ifndef _FILEITERATOR_H_
#define _FILEITERATOR_H_

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>

#define SD_CHIP_SELECT 4

namespace cpc_rom_board {

struct FileInfo {
    String name;
    uint32_t pos;

    operator bool() {
        return pos != 0;
    }
};

struct FileLayout {
    int write_error;             // Print
    unsigned long _timeout;      // Stream
    unsigned long _startMillis;  // Stream
    void * x;                    // unknown
    char _name[13];              // File
    SdFile *_file;               // File
};

class FileIterator {

public:
    FileIterator() = default;
    String getCurrentFilename();
    String getPreviousFilename();
    String getNextFilename();

    File getCurrentFile();
    File findFile(const char* filename);

    void setupSD();
    void destroy();
    void moveNext();
    void movePrevious();
    void reset();

private:
    File m_root;
    FileInfo m_prev;
    FileInfo m_current;
    FileInfo m_next;

    FileInfo getNextFileInfo(const FileLayout &f, uint32_t pos);
    FileInfo getPreviousFileInfo(const FileLayout &f, uint32_t pos);
    FileInfo _getNextFile();
    FileInfo _getPreviousFile();
};

}

#endif