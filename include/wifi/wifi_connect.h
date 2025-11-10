#ifndef WIFI_CONNECT_H
#define WIFI_CONNECT_H

#include <WiFi.h>
#include <M5Unified.h>

class WiFiConnect {
public:
    void begin(const char* ssid, const char* password);
    bool isConnected();
    IPAddress getIP();
    
private:
    const char* _ssid;
    const char* _password;
};

#endif
