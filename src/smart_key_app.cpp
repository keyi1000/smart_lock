#include "smart_key_app.h"
#include "wifi/wifi_connect.h"
#include "http/http_server.h"
#include "config.h"
#include <M5Unified.h>

WiFiConnect wifiConnect;
HTTPServer httpServer;

void SmartKeyApp::begin() {
    M5.begin();
    M5.Display.setRotation(1);
    M5.Display.setTextSize(2);
    
    // WiFi接続 (config.hから読み込み)
    wifiConnect.begin(WIFI_SSID, WIFI_PASSWORD);
    
    // HTTPサーバー起動
    httpServer.begin();
}

void SmartKeyApp::run() {
    M5.update();
    httpServer.handleClient();
    delay(10);
}