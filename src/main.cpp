#include <Arduino.h>
#include <SD.h>
#include <SPI.h>

#include "controller.h"
#include "log.h"

cpc_rom_board::Controller controller;

void setup() {
    setupSerialDebug();

    //local_storage::erase(true);
    controller.init();
}

void loop() {
    controller.loop();
}
