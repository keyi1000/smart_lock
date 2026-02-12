#ifndef BLE_CONNECTION_MANAGER_H
#define BLE_CONNECTION_MANAGER_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <vector>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

class RGBController;

class BLEConnectionManager {
public:
    void init(const String& serviceUuid);
    void startAdvertising();
    void stopAdvertising();
    bool isConnected();
    void sendNotification(const String& message);
    void setRGBController(RGBController* controller);  // RGBコントローラーを設定
    bool fetchAndSavePrivateKey();  // 秘密鍵を取得して保存
    void update();  // メインループから呼び出す更新処理
    bool verifyPublicKey(const String& receivedPublicKey);  // 公開鍵を検証
    
private:
    BLEServer* pServer = nullptr;
    BLECharacteristic* pCharacteristic = nullptr;
    bool deviceConnected = false;
    bool shouldRestartAdvertising = false;
    bool shouldFetchPrivateKey = false;
    bool shouldProcessUnlock = false;  // unlock処理フラグ
    String pendingPublicKey;  // 処理待ちの公開鍵
    String currentServiceUuid;
    uint32_t notifyCounter = 0;
    String receiveBuffer;  // 受信データのバッファ
    
    void restartAdvertising();
    void processCommand(const String& command);  // コマンド処理
    int base64Decode(const char* input, uint8_t* output, int maxLen);
    void base64Encode(char* output, const uint8_t* input, int len);
    
    friend class MyServerCallbacks;
    friend class MyCharacteristicCallbacks;
};

#endif
