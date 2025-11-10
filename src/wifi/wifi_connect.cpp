#include "wifi/wifi_connect.h"

void WiFiConnect::begin(const char* ssid, const char* password) {
    _ssid = ssid;
    _password = password;
    
    M5.Display.println("Connecting WiFi...");
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        M5.Display.print(".");
    }
    
    M5.Display.println("\nConnected!");
    M5.Display.print("IP: ");
    M5.Display.println(WiFi.localIP());
    
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
}

bool WiFiConnect::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

IPAddress WiFiConnect::getIP() {
    return WiFi.localIP();
}
