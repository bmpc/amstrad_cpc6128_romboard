
#include "fileiterator.h"
#include "log.h"

#define BUFFER_SIZE 20


namespace file_iterator {
namespace {
File m_root;
File m_current_entry;
String m_prev;
String m_current;
String m_next;

String m_filename_buffer[BUFFER_SIZE]; // ~ 200 bytes

String _getNextFilename() {
    while (true) {
        File entry = m_root.openNextFile();
        if (!entry) { // No more files
            return "";
        }

        if (!entry.isDirectory()) {
            String name = String(entry.name());
            entry.close();

            return name;
        }

        entry.close();
    }

    return "";
}

String _getPreviousFilename() {
    String prevEntryName = "";
    File entry;
    uint32_t pos = m_root.position();
    
    int prevEntryPos = pos;
    while (true) {
        prevEntryPos -= sizeof(dir_t);
        if (prevEntryPos < 0) {
            m_root.rewindDirectory();
            return "";
        }

        m_root.seek(prevEntryPos);

        entry = m_root.openNextFile();
        if (!entry) { // first file is a dir
            return "";
        }

        if (!entry.isDirectory()) {
            prevEntryName = entry.name();
            entry.close();
            return prevEntryName;
        }
        
        entry.close();
    }

    return "";
}
} // namespace anonymous

void setupSD() {
    if (!SD.begin(SD_CHIP_SELECT)) {
        DEBUG_PRINTLN("SD card initialization failed!");
        while (1)
            ;
    }

    DEBUG_PRINTLN("SD card initialization complete.");
}

void destroy() {
    m_root.close();
}

File findFile(String &filename) {
    File entry;
    
    reset();
    while (true) {
        entry = m_root.openNextFile();
        if (!entry) { // No more files
            return entry;
        }

        if (!entry.isDirectory()) {
            if (filename.equalsIgnoreCase(entry.name())) {
                return entry;
            }
        }

        entry.close();
    }

    return entry;
}

void moveNext() {
    if (m_current == "" || m_next == "") {
        reset();
        m_current = _getNextFilename();
        m_next = _getNextFilename();
    } else {
        m_prev = m_current;
        m_current = m_next;
        m_next = _getNextFilename();
    }
}

void movePrevious() {
    if (m_current == "" || m_prev == "") {
        reset();
        m_current = _getNextFilename();
        m_next = _getNextFilename();
    } else {
        m_next = m_current;
        m_current = m_prev;
        m_prev = _getPreviousFilename();
    }
}

void reset() {
    m_root.close();
    m_root = SD.open("/");

    m_current = "";
    m_prev = "";
    m_next = "";
}

String getCurrentFilename() {
    return m_current;
}

String getPreviousFilename() {
    return m_prev;
}

String getNextFilename() {
    return m_next;
}

File getCurrentFile() {
    File entry;
    m_root.rewindDirectory();
    while (true) {
        entry = m_root.openNextFile();
        if (!entry) { // No more files
            return entry;
        }

        if (!entry.isDirectory()) {
            if (m_current.equals(entry.name()))
                return entry;
        }

        entry.close();
    }

    return entry;
}

} // namespace file_iterator
