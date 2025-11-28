#include "smart_key_app.h"
#include "wifi/wifi_connect.h"
#include "http/http_server.h"
#include "config.h"
#include "mqtt/mqtt_manager.h"
#include <M5Unified.h>
#include <LittleFS.h>

WiFiConnect wifiConnect;
HTTPServer httpServer;
MqttManager mqttManager;

void SmartKeyApp::begin() {
    // シリアル通信初期化
    Serial.begin(115200);
    delay(1000);  // シリアル初期化待ち
    Serial.println("\n\n=== Smart Key App Starting ===");
    
    M5.begin();
    M5.Display.setRotation(1);
    M5.Display.setTextSize(2);
    
    Serial.println("Display initialized");
    
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
    Serial.println("WiFi connected!");
    // MQTT 初期化（オプション）
    Serial.println("Initializing MQTT...");
    mqttManager.begin(MQTT_BROKER, MQTT_PORT, MQTT_CLIENT_ID);
    mqttManager.setMessageHandler([](const char* topic, const uint8_t* payload, unsigned int length){
        Serial.print("MQTT received ["); Serial.print(topic); Serial.print("]: ");
        String msg;
        for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
        Serial.println(msg);
        // 受信コマンドに応じた処理をここに追加してください
    });
    
    // HTTPサーバー起動
    Serial.println("Starting HTTP server...");
    httpServer.begin();
    Serial.println("HTTP server started");
    
    // 送信先サーバーのURLと認証情報を設定
    Serial.println("Configuring target server...");
    httpServer.setTargetServer(
        SEND_SERVER_URL,
        SEND_SERVER_USER,
        SEND_SERVER_PASSWORD
    );
    Serial.println("=== Initialization Complete ===\n");
}

void SmartKeyApp::run() {
    M5.update();
    httpServer.handleClient();
    // MQTT ループ処理
    mqttManager.loop();

    // 例: 定期的にステータスメッセージを送信
    static unsigned long lastPub = 0;
    if (millis() - lastPub > 10000) {
        lastPub = millis();
        String payload = "online";
        if (LittleFS.exists("/shibata.jpg")) payload = "file_present";
        mqttManager.publish(MQTT_TOPIC_PUB, payload.c_str());
    }
    delay(10);
}