#include "ble/ble_uuid_manager.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <M5Unified.h>

bool BLEUuidManager::fetchAndSaveUuids(const char* apiUrl, const char* roomId) {
    Serial.println("\n=== Fetching BLE UUIDs from API ===");
    
    HTTPClient http;
    String url = String(apiUrl) + "?room_id=" + String(roomId);
    
    Serial.print("Requesting: ");
    Serial.println(url);
    
    M5.Display.clear();
    M5.Display.setCursor(0, 0);
    M5.Display.println("Fetching UUIDs...");
    
    http.begin(url);
    http.setTimeout(10000);  // 10秒タイムアウト
    
    int httpCode = http.GET();
    
    if (httpCode != 200) {
        Serial.printf("HTTP request failed: %d\n", httpCode);
        M5.Display.println("Request failed!");
        M5.Display.print("Code: ");
        M5.Display.println(httpCode);
        http.end();
        return false;
    }
    
    String payload = http.getString();
    http.end();
    
    Serial.println("Response received:");
    Serial.println(payload);
    
    // JSONパース
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);
    
    if (error) {
        Serial.print("JSON parse failed: ");
        Serial.println(error.c_str());
        M5.Display.println("Parse error!");
        return false;
    }
    
    // UUIDを取得
    uuids.clear();
    JsonArray bleUuidsArray = doc["ble_uuids"];
    
    if (bleUuidsArray.isNull() || bleUuidsArray.size() == 0) {
        Serial.println("No UUIDs found in response");
        M5.Display.println("No UUIDs found!");
        return false;
    }
    
    for (JsonVariant value : bleUuidsArray) {
        String uuid = value.as<String>();
        uuids.push_back(uuid);
        Serial.print("UUID found: ");
        Serial.println(uuid);
    }
    
    // ファイルに保存
    if (!saveUuidsToFile()) {
        Serial.println("Failed to save UUIDs to file");
        M5.Display.println("Save failed!");
        return false;
    }
    
    Serial.printf("✓ Successfully saved %d UUID(s)\n", uuids.size());
    M5.Display.println("\nUUIDs saved!");
    M5.Display.print(uuids.size());
    M5.Display.println(" UUID(s)");
    
    Serial.println("=== UUID Fetch Complete ===\n");
    return true;
}

bool BLEUuidManager::saveUuidsToFile() {
    File file = LittleFS.open(UUIDS_FILE_PATH, "w");
    if (!file) {
        Serial.println("Failed to open file for writing");
        return false;
    }
    
    for (const String& uuid : uuids) {
        file.println(uuid);
    }
    
    file.close();
    Serial.print("UUIDs saved to: ");
    Serial.println(UUIDS_FILE_PATH);
    return true;
}

bool BLEUuidManager::loadUuids() {
    Serial.println("Loading UUIDs from file...");
    
    if (!LittleFS.exists(UUIDS_FILE_PATH)) {
        Serial.println("UUID file does not exist");
        return false;
    }
    
    File file = LittleFS.open(UUIDS_FILE_PATH, "r");
    if (!file) {
        Serial.println("Failed to open UUID file");
        return false;
    }
    
    uuids.clear();
    while (file.available()) {
        String line = file.readStringUntil('\n');
        line.trim();
        if (line.length() > 0) {
            uuids.push_back(line);
            Serial.print("Loaded UUID: ");
            Serial.println(line);
        }
    }
    
    file.close();
    Serial.printf("✓ Loaded %d UUID(s)\n", uuids.size());
    return uuids.size() > 0;
}

std::vector<String> BLEUuidManager::getUuids() {
    return uuids;
}
