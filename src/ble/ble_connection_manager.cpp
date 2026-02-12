#include "ble/ble_connection_manager.h"
#include "rgb/rgb_controller.h"
#include "config.h"
#include <M5Unified.h>
#include <ArduinoJson.h>
#include <Ed25519.h>
#include <Crypto.h>

#define DEVICE_NAME "M5Stack-SmartLock"

// 外部からアクセスできるようにグローバル変数として定義
static BLEConnectionManager* g_bleManager = nullptr;
static RGBController* g_rgbController = nullptr;

// サーバーコールバック：接続・切断時の処理
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        if (g_bleManager) {
            g_bleManager->deviceConnected = true;
            // 接続時に秘密鍵取得フラグを立てる（実際の処理はメインループで実行）
            g_bleManager->shouldFetchPrivateKey = true;
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
            // 受信データをバッファに追加
            if (g_bleManager) {
                for (size_t i = 0; i < value.length(); i++) {
                    g_bleManager->receiveBuffer += value[i];
                }
                
                // 改行または終端文字を検出したら処理
                if (g_bleManager->receiveBuffer.indexOf('\n') >= 0 || 
                    g_bleManager->receiveBuffer.indexOf('\0') >= 0 ||
                    g_bleManager->receiveBuffer.endsWith("}")) {
                    
                    String completeMessage = g_bleManager->receiveBuffer;
                    completeMessage.trim();
                    g_bleManager->receiveBuffer = "";  // バッファをクリア
                    
                    Serial.print("RX (complete): ");
                    Serial.println(completeMessage);
                    
                    M5.Display.fillRect(0, 80, 320, 20, BLACK);
                    M5.Display.setCursor(10, 80);
                    M5.Display.setTextColor(CYAN);
                    M5.Display.print("RX: ");
                    M5.Display.println(completeMessage.substring(0, 30));
                    
                    // コマンド処理
                    g_bleManager->processCommand(completeMessage);
                }
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
    shouldFetchPrivateKey = false;
    shouldProcessUnlock = false;
    receiveBuffer = "";  // 受信バッファを初期化
    pendingPublicKey = "";
    
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

void BLEConnectionManager::processCommand(const String& message) {
    // PEM形式の公開鍵を検出（Ed25519 PUBLIC KEY）
    if (message.indexOf("-----BEGIN ED25519 PUBLIC KEY-----") >= 0) {
        Serial.println("✓ Ed25519 Public Key received - preparing unlock");
        
        // PEM形式から公開鍵データを抽出
        String publicKey = message;
        publicKey.replace("-----BEGIN ED25519 PUBLIC KEY-----", "");
        publicKey.replace("-----END ED25519 PUBLIC KEY-----", "");
        publicKey.replace("\n", "");
        publicKey.replace("\r", "");
        publicKey.trim();
        
        Serial.print("Public key (Base64): ");
        Serial.println(publicKey);
        
        // メインループで処理するためにフラグを立てる
        pendingPublicKey = publicKey;
        shouldProcessUnlock = true;
        
        return;
    }
    
    // JSONとして解析を試みる
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, message);
    
    if (!error && doc["command"].is<String>()) {
        String command = doc["command"].as<String>();
        
        // "lock"コマンドを受信した場合
        if (command == "lock") {
            Serial.println("✓ Lock command received! Activating RGB Unit (RED)...");
            
            M5.Display.fillRect(0, 120, 320, 20, BLACK);
            M5.Display.setCursor(10, 120);
            M5.Display.setTextColor(RED);
            M5.Display.println("LOCKED!");
            
            if (g_rgbController != nullptr) {
                g_rgbController->lockEffect();
            }
            
            M5.Display.fillRect(0, 120, 320, 20, BLACK);
        }
        // "unlock"コマンドを受信した場合、公開鍵を検証
        else if (command == "unlock") {
            if (doc["public_key"].is<String>()) {
                String publicKey = doc["public_key"].as<String>();
                Serial.println("✓ Unlock command with public key received");
                Serial.print("Public key: ");
                Serial.println(publicKey);
                
                // メインループで処理するためにフラグを立てる
                pendingPublicKey = publicKey;
                shouldProcessUnlock = true;
            } else {
                Serial.println("✗ Unlock command without public key!");
            }
        }
    }
    // 後方互換性: 従来の文字列コマンドもサポート
    else if (message == "lock") {
        Serial.println("✓ Lock command (legacy) received!");
        
        M5.Display.fillRect(0, 120, 320, 20, BLACK);
        M5.Display.setCursor(10, 120);
        M5.Display.setTextColor(RED);
        M5.Display.println("LOCKED!");
        
        if (g_rgbController != nullptr) {
            g_rgbController->lockEffect();
        }
        
        M5.Display.fillRect(0, 120, 320, 20, BLACK);
    }
    else if (message == "unlock") {
        Serial.println("⚠ Unlock command (legacy) received - no authentication!");
    }
}

void BLEConnectionManager::update() {
    // 秘密鍵取得フラグがtrueの場合、処理を実行
    if (shouldFetchPrivateKey) {
        shouldFetchPrivateKey = false;
        fetchAndSavePrivateKey();
    }
    
    // unlock処理フラグがtrueの場合、検証と開錠を実行
    if (shouldProcessUnlock) {
        shouldProcessUnlock = false;
        
        Serial.println("Processing unlock request in main loop...");
        
        // 公開鍵を検証
        if (verifyPublicKey(pendingPublicKey)) {
            Serial.println("✓ Public key verified! Unlocking...");
            
            M5.Display.fillRect(0, 120, 320, 20, BLACK);
            M5.Display.setCursor(10, 120);
            M5.Display.setTextColor(BLUE);
            M5.Display.println("UNLOCKED!");
            
            if (g_rgbController != nullptr) {
                g_rgbController->unlockEffect();
            }
            
            delay(2000);  // 効果を表示
            M5.Display.fillRect(0, 120, 320, 20, BLACK);
        } else {
            Serial.println("✗ Public key verification failed!");
            
            M5.Display.fillRect(0, 120, 320, 20, BLACK);
            M5.Display.setCursor(10, 120);
            M5.Display.setTextColor(RED);
            M5.Display.println("AUTH FAILED!");
            delay(2000);
            M5.Display.fillRect(0, 120, 320, 20, BLACK);
        }
        
        pendingPublicKey = "";  // クリア
    }
}

bool BLEConnectionManager::fetchAndSavePrivateKey() {
    Serial.println("\n=== Fetching Private Key from API ===");
    
    HTTPClient http;
    String url = String(key_generate_server) + "api/keys/" + String(ROOM_NAME) + "/private";
    
    Serial.print("Requesting: ");
    Serial.println(url);
    
    http.begin(url);
    http.setTimeout(10000);  // 10秒タイムアウト
    
    int httpCode = http.GET();
    
    if (httpCode != 200) {
        Serial.printf("HTTP request failed: %d\n", httpCode);
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
        return false;
    }
    
    // private_keyを取得
    if (!doc["private_key"].is<String>()) {
        Serial.println("No private_key found in response");
        return false;
    }
    
    String privateKey = doc["private_key"].as<String>();
    String roomNumber = doc["room_number"].as<String>();
    String publicKey = doc["public_key"].is<String>() ? doc["public_key"].as<String>() : "";
    
    Serial.println("Private key received:");
    Serial.println(privateKey);
    Serial.print("Room number: ");
    Serial.println(roomNumber);
    if (publicKey.length() > 0) {
        Serial.println("Public key received:");
        Serial.println(publicKey);
    }
    
    // LittleFSに保存
    if (!LittleFS.begin(true)) {
        Serial.println("Failed to mount LittleFS");
        return false;
    }
    
    // 秘密鍵を保存
    File file = LittleFS.open("/private_key.pem", "w");
    if (!file) {
        Serial.println("Failed to open file for writing");
        LittleFS.end();
        return false;
    }
    file.print(privateKey);
    file.close();
    
    // 公開鍵を保存（検証用）
    if (publicKey.length() > 0) {
        file = LittleFS.open("/public_key.pem", "w");
        if (file) {
            file.print(publicKey);
            file.close();
            Serial.println("✓ Public key saved");
        }
    }
    
    // 部屋番号を保存（公開鍵検証用）
    file = LittleFS.open("/room_number.txt", "w");
    if (file) {
        file.print(roomNumber);
        file.close();
    }
    
    LittleFS.end();
    
    Serial.println("✓ Private key and room number saved");
    Serial.println("=== Private Key Fetch Complete ===\n");
    
    return true;
}

bool BLEConnectionManager::verifyPublicKey(const String& receivedPublicKey) {
    Serial.println("\n=== Verifying Public Key ===");
    Serial.print("Received public key (Base64, length: ");
    Serial.print(receivedPublicKey.length());
    Serial.println("):");
    Serial.println(receivedPublicKey);
    
    // LittleFSから秘密鍵を読み込む
    if (!LittleFS.begin(true)) {
        Serial.println("Failed to mount LittleFS");
        return false;
    }
    
    if (!LittleFS.exists("/private_key.pem")) {
        Serial.println("Private key file not found");
        LittleFS.end();
        return false;
    }
    
    File file = LittleFS.open("/private_key.pem", "r");
    if (!file) {
        Serial.println("Failed to open private key file");
        LittleFS.end();
        return false;
    }
    
    String privateKeyPem = file.readString();
    file.close();
    LittleFS.end();
    
    Serial.println("Private key loaded from file");
    
    // PEM形式から秘密鍵のBase64部分を抽出
    privateKeyPem.replace("-----BEGIN ED25519 PRIVATE KEY-----", "");
    privateKeyPem.replace("-----END ED25519 PRIVATE KEY-----", "");
    privateKeyPem.replace("\n", "");
    privateKeyPem.replace("\r", "");
    privateKeyPem.replace(" ", "");
    privateKeyPem.trim();
    
    Serial.print("Private key Base64 (length: ");
    Serial.print(privateKeyPem.length());
    Serial.println(")");
    
    // Base64デコード
    uint8_t privateKeyBytes[64];  // Ed25519秘密鍵は64バイト
    int decodedLen = base64Decode(privateKeyPem.c_str(), privateKeyBytes, 64);
    
    if (decodedLen < 32) {
        Serial.printf("Failed to decode private key (got %d bytes, need at least 32)\n", decodedLen);
        return false;
    }
    
    Serial.printf("Private key decoded: %d bytes\n", decodedLen);
    
    // 秘密鍵の最初の32バイトから公開鍵を導出
    uint8_t derivedPublicKeyBytes[32];
    Ed25519::derivePublicKey(derivedPublicKeyBytes, privateKeyBytes);
    
    Serial.println("Public key derived from private key");
    
    // 導出した公開鍵をBase64エンコード
    char derivedPublicKeyBase64[64];
    base64Encode(derivedPublicKeyBase64, derivedPublicKeyBytes, 32);
    
    Serial.print("Derived public key (Base64): ");
    Serial.println(derivedPublicKeyBase64);
    
    // 受信した公開鍵と比較
    bool isValid = (receivedPublicKey == String(derivedPublicKeyBase64));
    
    if (isValid) {
        Serial.println("✓ Public key verification succeeded!");
    } else {
        Serial.println("✗ Public key verification failed!");
        Serial.println("Keys do not match");
    }
    
    Serial.println("=== Public Key Verification Complete ===\n");
    
    return isValid;
}

// Base64デコード用のヘルパー関数
int BLEConnectionManager::base64Decode(const char* input, uint8_t* output, int maxLen) {
    const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int i = 0, j = 0, len = strlen(input);
    uint32_t buffer = 0;
    int bits = 0;
    
    for (i = 0; i < len && j < maxLen; i++) {
        if (input[i] == '=') break;
        if (input[i] == '\n' || input[i] == '\r' || input[i] == ' ') continue;
        
        const char* pos = strchr(base64_chars, input[i]);
        if (!pos) continue;
        
        buffer = (buffer << 6) | (pos - base64_chars);
        bits += 6;
        
        if (bits >= 8) {
            bits -= 8;
            output[j++] = (buffer >> bits) & 0xFF;
        }
    }
    
    return j;
}

// Base64エンコード用のヘルパー関数
void BLEConnectionManager::base64Encode(char* output, const uint8_t* input, int len) {
    const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int i = 0, j = 0;
    uint32_t buffer = 0;
    int bits = 0;
    
    for (i = 0; i < len; i++) {
        buffer = (buffer << 8) | input[i];
        bits += 8;
        
        while (bits >= 6) {
            bits -= 6;
            output[j++] = base64_chars[(buffer >> bits) & 0x3F];
        }
    }
    
    if (bits > 0) {
        buffer <<= (6 - bits);
        output[j++] = base64_chars[buffer & 0x3F];
    }
    
    while (j % 4 != 0) {
        output[j++] = '=';
    }
    
    output[j] = '\0';
}
