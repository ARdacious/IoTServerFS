// Filesystem stuff
#include <FS.h>

#include "Config.h"


void format() {
  if (SPIFFS.begin()) {
    SPIFFS.format();
    SPIFFS.end();
  }
}

void loadConfig() {
  if (SPIFFS.begin()) {
    if (SPIFFS.exists("/config.bin")) {
      File f = SPIFFS.open("/config.bin", "r");
      if (f) {
        size_t size = f.size();
        if (size == sizeof(cfg)) {
          f.readBytes((char*)&cfg, size);
          f.close();
        }
      }
    }
    SPIFFS.end();
  }
}

void saveConfig() {
  if (SPIFFS.begin()) {
    File f = SPIFFS.open("/config.bin","w");
    if (f) {
      f.write((uint8_t*)&cfg, sizeof(cfg));
      f.close();
    }
    SPIFFS.end();
  }
}

