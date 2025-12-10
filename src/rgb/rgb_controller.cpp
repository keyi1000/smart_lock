#include "rgb/rgb_controller.h"
#include "config.h"
#include <M5Unified.h>

void RGBController::init(uint8_t dataPin, uint8_t numLedsParam, uint8_t brightness) {
    numLeds = numLedsParam;
    leds = new CRGB[numLeds];
    
    // FastLED初期化
    FastLED.addLeds<WS2812, RGB_DATA_PIN, GRB>(leds, numLeds);
    FastLED.setBrightness(brightness);
    
    Serial.println("\n=== RGB Unit Initialized ===");
    Serial.print("Data Pin: ");
    Serial.println(dataPin);
    Serial.print("Number of LEDs: ");
    Serial.println(numLeds);
    Serial.print("Brightness: ");
    Serial.println(brightness);
    
    // 初期化時のテスト
    clear();
    show();
}

void RGBController::setColor(uint8_t index, uint32_t color) {
    if (index < numLeds) {
        uint8_t r = (color >> 16) & 0xFF;
        uint8_t g = (color >> 8) & 0xFF;
        uint8_t b = color & 0xFF;
        leds[index] = CRGB(r, g, b);
    }
}

void RGBController::setColorRGB(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    if (index < numLeds) {
        leds[index] = CRGB(r, g, b);
    }
}

void RGBController::setBrightness(uint8_t brightness) {
    FastLED.setBrightness(brightness);
    FastLED.show();
}

void RGBController::clear() {
    for (uint8_t i = 0; i < numLeds; i++) {
        leds[i] = CRGB::Black;
    }
}

void RGBController::show() {
    FastLED.show();
}

void RGBController::lockEffect() {
    Serial.println("=== Lock Effect Started ===");
    
    // 全LEDを赤色に点灯（常時点灯）
    for (uint8_t i = 0; i < numLeds; i++) {
        setColorRGB(i, 255, 0, 0);  // 赤色
    }
    show();
    
    Serial.println("=== Lock Effect Completed (RED ON) ===");
}

void RGBController::unlockEffect() {
    Serial.println("=== Unlock Effect Started ===");
    
    // 全LEDを青色に点灯（常時点灯）
    for (uint8_t i = 0; i < numLeds; i++) {
        setColorRGB(i, 0, 0, 255);  // 青色
    }
    show();
    
    Serial.println("=== Unlock Effect Completed (BLUE ON) ===");
}
