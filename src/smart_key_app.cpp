#include "smart_key_app.h"
#include "wifi/wifi_connect.h"
#include "http/http_server.h"
#include "ble/ble_uuid_manager.h"
#include "ble/ble_connection_manager.h"
#include "rgb/rgb_controller.h"
#include "config.h"
#include <M5Unified.h>
#include <LittleFS.h>

WiFiConnect wifiConnect;
HTTPServer httpServer;
BLEUuidManager bleUuidManager;
BLEConnectionManager bleConnectionManager;
RGBController rgbController;

void SmartKeyApp::begin() {
    // シリアル通信初期化
    Serial.begin(115200);
    delay(1000);  // シリアル初期化待ち
    Serial.println("\n\n=== Smart Key App Starting ===");
    
    M5.begin();
    M5.Display.setRotation(1);
    M5.Display.setTextSize(2);
    
    Serial.println("Display initialized");
    
    // RGBユニット初期化
    Serial.println("Initializing RGB Unit...");
    rgbController.init(RGB_DATA_PIN, RGB_NUM_LEDS, RGB_LED_BRIGHTNESS);
    Serial.println("✓ RGB Unit initialized");
    
    // LittleFS初期化
    Serial.println("Mounting LittleFS...");
    if (!LittleFS.begin(true)) {
        Serial.println("✗ LittleFS mount failed!");
        M5.Display.println("FS Error!");
    } else {
        Serial.println("✓ LittleFS mounted");
        
        // ファイルの存在確認
        if (LittleFS.exists("/shibata.jpg")) {
            File f = LittleFS.open("/shibata.jpg", "r");
            Serial.print("shibata.jpg found: ");
            Serial.print(f.size());
            Serial.println(" bytes");
            f.close();
        } else {
            Serial.println("⚠ shibata.jpg not found!");
        }
    }
    
    // WiFi接続 (config.hから読み込み)
    Serial.println("Starting WiFi connection...");
    wifiConnect.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.println("WiFi connected!");
    
    // BLE UUIDを取得して保存
    Serial.println("Fetching BLE UUIDs...");
    bool uuidFetched = bleUuidManager.fetchAndSaveUuids(BLE_UUID_API_URL, ROOM_ID);
    if (uuidFetched) {
        Serial.println("✓ BLE UUIDs fetched and saved");
    } else {
        Serial.println("⚠ Failed to fetch BLE UUIDs, trying to load from file...");
        if (bleUuidManager.loadUuids()) {
            Serial.println("✓ BLE UUIDs loaded from file");
        } else {
            Serial.println("✗ No BLE UUIDs available");
        }
    }
    
    // BLE初期化（Peripheralモード）
    std::vector<String> uuids = bleUuidManager.getUuids();
    if (uuids.size() > 0) {
        // 最初のUUIDをService UUIDとして使用
        bleConnectionManager.init(uuids[0]);
        bleConnectionManager.setRGBController(&rgbController);  // RGBコントローラーを設定
        bleConnectionManager.startAdvertising();
        
        Serial.println("✓ BLE Peripheral started");
        Serial.println("Waiting for Central to connect...");
        
        M5.Display.clear();
        M5.Display.setCursor(0, 0);
        M5.Display.println("BLE Peripheral");
        M5.Display.setCursor(10, 40);
        M5.Display.setTextColor(YELLOW);
        M5.Display.println("Status: Advertising");
        
        delay(2000);  // 結果表示のため待機
    } else {
        Serial.println("⚠ Skipping BLE (no UUIDs)");
    }
    
    // HTTPサーバー起動
    Serial.println("Starting HTTP server...");
    httpServer.begin();
    Serial.println("HTTP server started");
    
    Serial.println("=== Initialization Complete ===\n");
}

void SmartKeyApp::run() {
    M5.update();
    httpServer.handleClient();
    
    // BLE接続管理：切断後の広告再開処理
    static bool oldDeviceConnected = false;
    bool currentConnected = bleConnectionManager.isConnected();
    
    // 切断された場合
    if (!currentConnected && oldDeviceConnected) {
        delay(500);  // 安定のため待機
        bleConnectionManager.startAdvertising();
        Serial.println("Restarted advertising after disconnect");
    }
    
    // 新規接続された場合
    if (currentConnected && !oldDeviceConnected) {
        Serial.println("New connection established");
    }
    
    oldDeviceConnected = currentConnected;
    
    // 接続中は2秒ごとにNotifyを送信
    static unsigned long lastNotifyTime = 0;
    if (currentConnected && (millis() - lastNotifyTime > 2000)) {
        lastNotifyTime = millis();
        
        char msg[32];
        static uint32_t notifyCounter = 0;
        snprintf(msg, sizeof(msg), "ping %lu", notifyCounter++);
        bleConnectionManager.sendNotification(msg);
    }
    
    delay(10);
}