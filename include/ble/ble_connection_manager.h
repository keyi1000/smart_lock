#ifndef BLE_CONNECTION_MANAGER_H
#define BLE_CONNECTION_MANAGER_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <vector>

class RGBController;

class BLEConnectionManager {
public:
    void init(const String& serviceUuid);
    void startAdvertising();
    void stopAdvertising();
    bool isConnected();
    void sendNotification(const String& message);
    void setRGBController(RGBController* controller);  // RGBコントローラーを設定
    
private:
    BLEServer* pServer = nullptr;
    BLECharacteristic* pCharacteristic = nullptr;
    bool deviceConnected = false;
    bool shouldRestartAdvertising = false;
    String currentServiceUuid;
    uint32_t notifyCounter = 0;
    
    void restartAdvertising();
    
    friend class MyServerCallbacks;
    friend class MyCharacteristicCallbacks;
};

#endif
