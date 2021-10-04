#include <Arduino.h>
#include <SD.h>
#include <SPI.h>

#include "controller.h"
#include "log.h"

void setup() {
    setupSerialDebug();

    //local_storage::erase(true);
    //local_storage::erase(false);
    controller::init();

    /*
  delay(2000);
  interrupt_browseNext();
  delay(2000);
  interrupt_browseNext();
  delay(2000);
  interrupt_browseNext();
  delay(2000);
  interrupt_browseNext();
  delay(2000);
  interrupt_browseNext();
  delay(2000);
  interrupt_confirm();
  delay(2000);
  interrupt_cancel();
  delay(2000);
  interrupt_browseNext();
  delay(2000);
  interrupt_browseNext();
  delay(2000);
  interrupt_browseNext();
  delay(2000);
  interrupt_confirm();
  delay(2000);
  interrupt_confirm();
  */
}

void loop() {
    controller::loop();
}
