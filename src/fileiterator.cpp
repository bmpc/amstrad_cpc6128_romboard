
#include "fileiterator.h"
#include "log.h"

namespace file_iterator {
namespace {
File m_root;
File m_current_entry;
String m_prev;
String m_current;
String m_next;

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
