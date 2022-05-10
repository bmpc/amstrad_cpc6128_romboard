
#include "fileiterator.h"
#include "log.h"

namespace file_iterator {
namespace {

struct FileInfo {
    String name;
    uint32_t pos;

    operator bool() {
        return pos != 0;
    }
};

// too fucking dangerous
struct FileLayout {
    int write_error;             // Print
    unsigned long _timeout;      // Stream
    unsigned long _startMillis;  // Stream
    void * x;                    // unknown
    char _name[13];              // File
    SdFile *_file;               // File
};

File m_root;

FileInfo m_prev;
FileInfo m_current;
FileInfo m_next;

FileInfo getNextFileInfo(const FileLayout &f, uint32_t pos) {
  dir_t p;

  if (pos == 0) {
    f._file->rewind();
  } else {
    f._file->seekSet(pos);
  }

  while (f._file->readDir(&p) > 0) {
    // done if past last used entry
    if (p.name[0] == DIR_NAME_FREE) {
      return FileInfo();
    }

    // skip deleted entry and entries for . and  ..
    if (p.name[0] == DIR_NAME_DELETED || p.name[0] == '.') {
      continue;
    }

    if (DIR_IS_FILE(&p)) {
      // print file name with possible blank fill
      char name[13];
      f._file->dirName(p, name);

      return FileInfo{name, f._file->curPosition() - sizeof(dir_t) };
    }
  }

  return FileInfo();
}

FileInfo getPreviousFileInfo(const FileLayout &f, uint32_t pos) {
  dir_t p;

  int currentPos = pos - sizeof(dir_t);
  f._file->seekSet(currentPos);

  while (f._file->readDir(&p) > 0) {
    // done if past last used entry
    if (p.name[0] == DIR_NAME_FREE) {
      return FileInfo();
    }

    if (DIR_IS_FILE(&p) && !(p.name[0] == DIR_NAME_DELETED || p.name[0] == '.') && f._file->curPosition() < pos) {
      // print file name with possible blank fill
      char name[13];
      f._file->dirName(p, name);

      return {name, (uint32_t)(f._file->curPosition() - sizeof(dir_t))};
    }

    currentPos -= sizeof(dir_t); // not yet ...
    f._file->seekSet(currentPos);
    
  }

  return FileInfo();
}

FileInfo _getNextFile() {
    int pos;
    if (!m_current) {
        pos = 0;
    } else {
        if (m_next) {
            pos = m_next.pos + sizeof(dir_t);
        } else {
            pos = m_current.pos + sizeof(dir_t);
        }
    }

    FileLayout * myFile = reinterpret_cast<FileLayout*>(&m_root);
    return getNextFileInfo(*myFile, pos);
}

FileInfo _getPreviousFile() {
    if (!m_current || !m_prev) {
        return FileInfo();
    }

    FileLayout * myFile = reinterpret_cast<FileLayout*>(&m_root);
    return getPreviousFileInfo(*myFile, m_prev.pos);
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
    if (!m_current || !m_next) {
        reset();
        m_current = _getNextFile();
        m_next = _getNextFile();
    } else {
        m_prev = m_current;
        m_current = m_next;
        m_next = _getNextFile();
    }
}

void movePrevious() {
    if (!m_current || !m_prev) {
        reset();
        m_current = _getNextFile();
        m_next = _getNextFile();
    } else {
        m_next = m_current;
        m_current = m_prev;
        m_prev = _getPreviousFile();
    }
}

void reset() {
    m_root.close();
    m_root = SD.open("/");

    m_current = FileInfo();
    m_prev = FileInfo();
    m_next = FileInfo();
}

String getCurrentFilename() {
    return m_current.name;
}

String getPreviousFilename() {
    return m_prev.name;
}

String getNextFilename() {
    return m_next.name;
}

File getCurrentFile() {
    char name[13];
    m_current.name.toCharArray(name, 13);
    return SD.open(name, FILE_READ);
}

} // namespace file_iterator
