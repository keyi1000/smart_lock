#ifndef BLE_UUID_MANAGER_H
#define BLE_UUID_MANAGER_H

#include <Arduino.h>
#include <vector>

class BLEUuidManager {
public:
    bool fetchAndSaveUuids(const char* apiUrl, const char* roomId);
    bool loadUuids();
    std::vector<String> getUuids();
    
private:
    std::vector<String> uuids;
    const char* UUIDS_FILE_PATH = "/ble_uuids.txt";
    
    bool saveUuidsToFile();
};

#endif
