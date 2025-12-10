#include "ble/ble_connection_manager.h"
#include "rgb/rgb_controller.h"
#include "config.h"
#include <M5Unified.h>

#define DEVICE_NAME "M5Stack-SmartLock"

// 外部からアクセスできるようにグローバル変数として定義
static BLEConnectionManager* g_bleManager = nullptr;
static RGBController* g_rgbController = nullptr;

// サーバーコールバック：接続・切断時の処理
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        if (g_bleManager) {
            g_bleManager->deviceConnected = true;
        }
        Serial.println("Central connected");
        
        M5.Display.fillRect(0, 40, 320, 20, BLACK);
        M5.Display.setCursor(10, 40);
        M5.Display.setTextColor(GREEN);
        M5.Display.println("Status: Connected");
    }

    void onDisconnect(BLEServer* pServer) {
        if (g_bleManager) {
            g_bleManager->deviceConnected = false;
            g_bleManager->shouldRestartAdvertising = true;
        }
        Serial.println("Central disconnected");
        
        M5.Display.fillRect(0, 40, 320, 20, BLACK);
        M5.Display.setCursor(10, 40);
        M5.Display.setTextColor(YELLOW);
        M5.Display.println("Status: Disconnected");
    }
};

// キャラクタリスティックコールバック：書き込み時の処理
class MyCharacteristicCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        
        if (value.length() > 0) {
            Serial.print("RX: ");
            Serial.println(value.c_str());
            
            M5.Display.fillRect(0, 80, 320, 20, BLACK);
            M5.Display.setCursor(10, 80);
            M5.Display.setTextColor(CYAN);
            M5.Display.print("RX: ");
            M5.Display.println(value.c_str());
            
            // "lock"コマンドを受信した場合、RGBユニットを赤色で光らせる
            if (value == "lock") {
                Serial.println("✓ Lock command received! Activating RGB Unit (RED)...");
                
                M5.Display.fillRect(0, 120, 320, 20, BLACK);
                M5.Display.setCursor(10, 120);
                M5.Display.setTextColor(RED);
                M5.Display.println("LOCKED!");
                
                // RGBユニットのロックエフェクトを実行
                if (g_rgbController != nullptr) {
                    g_rgbController->lockEffect();
                }
                
                M5.Display.fillRect(0, 120, 320, 20, BLACK);
            }
            // "unlock"コマンドを受信した場合、RGBユニットを青色で光らせる
            else if (value == "unlock") {
                Serial.println("✓ Unlock command received! Activating RGB Unit (BLUE)...");
                
                M5.Display.fillRect(0, 120, 320, 20, BLACK);
                M5.Display.setCursor(10, 120);
                M5.Display.setTextColor(BLUE);
                M5.Display.println("UNLOCKED!");
                
                // RGBユニットのアンロックエフェクトを実行
                if (g_rgbController != nullptr) {
                    g_rgbController->unlockEffect();
                }
                
                M5.Display.fillRect(0, 120, 320, 20, BLACK);
            }
        }
    }
};

void BLEConnectionManager::init(const String& serviceUuid) {
    Serial.println("\n=== Initializing BLE (Peripheral Mode) ===");
    
    g_bleManager = this;
    currentServiceUuid = serviceUuid;
    deviceConnected = false;
    shouldRestartAdvertising = false;
    
    // BLEデバイス初期化
    BLEDevice::init(DEVICE_NAME);
    
    // BLEサーバー作成
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    
    // BLEサービス作成
    BLEService* pService = pServer->createService(serviceUuid.c_str());
    
    // BLEキャラクタリスティック作成
    pCharacteristic = pService->createCharacteristic(
        BLE_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ   |
        BLECharacteristic::PROPERTY_WRITE  |
        BLECharacteristic::PROPERTY_NOTIFY
    );
    
    // ディスクリプタ追加（Notify用）
    pCharacteristic->addDescriptor(new BLE2902());
    pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
    
    // 初期値設定
    pCharacteristic->setValue("hello");
    
    // サービス開始
    pService->start();
    
    Serial.println("✓ BLE initialized as Peripheral");
    Serial.print("Service UUID: ");
    Serial.println(serviceUuid);
    Serial.print("Device name: ");
    Serial.println(DEVICE_NAME);
}

void BLEConnectionManager::startAdvertising() {
    Serial.println("Starting BLE advertising...");
    
    // 広告開始
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(currentServiceUuid.c_str());
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // iPhone接続の問題対策
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    
    Serial.println("✓ BLE advertising started");
    
    M5.Display.fillRect(0, 60, 320, 20, BLACK);
    M5.Display.setCursor(10, 60);
    M5.Display.setTextColor(MAGENTA);
    M5.Display.println("Status: Advertising");
}

void BLEConnectionManager::stopAdvertising() {
    Serial.println("Stopping BLE advertising...");
    BLEDevice::getAdvertising()->stop();
    Serial.println("✓ BLE advertising stopped");
}

void BLEConnectionManager::restartAdvertising() {
    delay(500);  // 安定のため少し待つ
    BLEDevice::startAdvertising();
    Serial.println("✓ Advertising restarted");
    
    M5.Display.fillRect(0, 60, 320, 20, BLACK);
    M5.Display.setCursor(10, 60);
    M5.Display.setTextColor(MAGENTA);
    M5.Display.println("Advertising restarted");
}

bool BLEConnectionManager::isConnected() {
    return deviceConnected;
}

void BLEConnectionManager::sendNotification(const String& message) {
    if (deviceConnected && pCharacteristic != nullptr) {
        pCharacteristic->setValue(message.c_str());
        pCharacteristic->notify();
        
        Serial.print("Notify: ");
        Serial.println(message);
        
        M5.Display.fillRect(0, 100, 320, 20, BLACK);
        M5.Display.setCursor(10, 100);
        M5.Display.setTextColor(GREEN);
        M5.Display.print("TX: ");
        M5.Display.println(message);
    }
}

void BLEConnectionManager::setRGBController(RGBController* controller) {
    g_rgbController = controller;
    Serial.println("✓ RGB Controller set");
}
