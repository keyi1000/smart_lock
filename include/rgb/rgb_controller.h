#ifndef RGB_CONTROLLER_H
#define RGB_CONTROLLER_H

#include <Arduino.h>
#include <FastLED.h>

class RGBController {
public:
    void init(uint8_t dataPin, uint8_t numLeds, uint8_t brightness);
    void setColor(uint8_t index, uint32_t color);
    void setColorRGB(uint8_t index, uint8_t r, uint8_t g, uint8_t b);
    void setBrightness(uint8_t brightness);
    void clear();
    void show();
    void lockEffect();    // ロック時のエフェクト（赤色）
    void unlockEffect();  // アンロック時のエフェクト（青色）
    
private:
    CRGB* leds = nullptr;
    uint8_t numLeds = 0;
};

#endif
