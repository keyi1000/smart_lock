#include "wifi/wifi_connect.h"

void WiFiConnect::begin(const char* ssid, const char* password) {
    _ssid = ssid;
    _password = password;
    
    Serial.println("--- WiFi Connection Start ---");
    Serial.print("SSID: ");
    Serial.println(ssid);
    
    M5.Display.println("Connecting WiFi...");
    M5.Display.print("SSID: ");
    M5.Display.println(ssid);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        M5.Display.print(".");
        Serial.print(".");
        attempts++;
        if (attempts > 60) {  // 30秒でタイムアウト
            Serial.println("\nWiFi connection timeout!");
            M5.Display.println("\nTimeout!");
            return;
        }
    }
    
    M5.Display.println("\nConnected!");
    M5.Display.print("IP: ");
    M5.Display.println(WiFi.localIP());
    
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Gateway: ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("DNS: ");
    Serial.println(WiFi.dnsIP());
    Serial.println("--- WiFi Connection Complete ---\n");
}

bool WiFiConnect::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

IPAddress WiFiConnect::getIP() {
    return WiFi.localIP();
}
