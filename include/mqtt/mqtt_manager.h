// mqtt_manager.h
#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>
#include <functional>

class MqttManager {
public:
    MqttManager();
    void begin(const char* broker, uint16_t port, const char* clientId = nullptr);
    void loop();
    bool publish(const char* topic, const char* payload);
    bool subscribe(const char* topic);
    void setMessageHandler(std::function<void(const char*, const uint8_t*, unsigned int)> handler);

private:
    // pimpl-like forward declarations are handled in cpp
    struct Impl;
    Impl* impl;
};

#endif // MQTT_MANAGER_H
