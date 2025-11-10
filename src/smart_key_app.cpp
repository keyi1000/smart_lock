#include "smart_key_app.h"
#include <M5Unified.h>

void SmartKeyApp::begin() {
    M5.begin();
    M5.Display.println("App Started");
}

void SmartKeyApp::run() {
    M5.update();
    delay(100);
}