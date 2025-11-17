#include "file/file_sender.h"
#include <M5Unified.h>

void FileSender::setTargetServer(const char* url) {
    targetServerUrl = url;
    Serial.print("Target server: ");
    Serial.println(targetServerUrl);
}

void FileSender::setAuth(const char* username, const char* password) {
    authUsername = username;
    authPassword = password;
    Serial.println("Auth credentials set");
}

bool FileSender::sendFile(const uint8_t* data, size_t size, FileType type, const char* filename) {
    Serial.println("=== FileSender::sendFile() START ===");
    
    if (targetServerUrl.isEmpty()) {
        Serial.println("ERROR: No target server URL set!");
        M5.Display.clear();
        M5.Display.setCursor(0, 0);
        M5.Display.println("Error:");
        M5.Display.println("No server URL");
        return false;
    }
    
    Serial.print("Target URL: ");
    Serial.println(targetServerUrl);
    
    HTTPClient http;
    
    Serial.print("Sending file: ");
    Serial.print(filename);
    Serial.print(" (");
    Serial.print(size);
    Serial.println(" bytes)");
    
    M5.Display.clear();
    M5.Display.setCursor(0, 0);
    M5.Display.println("Sending...");
    M5.Display.print("File: ");
    M5.Display.println(filename);
    M5.Display.print("Size: ");
    M5.Display.print(size);
    M5.Display.println(" bytes");
    
    // HTTPクライアント設定
    Serial.println("Calling http.begin()...");
    bool beginSuccess = http.begin(targetServerUrl);
    Serial.print("http.begin() result: ");
    Serial.println(beginSuccess ? "SUCCESS" : "FAILED");
    
    if (!beginSuccess) {
        Serial.println("ERROR: HTTPClient.begin() failed!");
        M5.Display.println("\nHTTP begin failed!");
        return false;
    }
    if (!beginSuccess) {
        Serial.println("ERROR: HTTPClient.begin() failed!");
        M5.Display.println("\nHTTP begin failed!");
        return false;
    }
    
    // Basic認証
    if (!authUsername.isEmpty() && !authPassword.isEmpty()) {
        http.setAuthorization(authUsername.c_str(), authPassword.c_str());
        Serial.print("Using Basic Auth - User: ");
        Serial.println(authUsername);
    } else {
        Serial.println("No authentication configured");
    }
    
    String contentType = getContentType(type);
    Serial.print("Content-Type: ");
    Serial.println(contentType);
    
    http.addHeader("Content-Type", contentType);
    http.addHeader("Content-Disposition", "attachment; filename=\"" + String(filename) + "\"");
    http.addHeader("X-File-Type", type == FileType::IMAGE ? "image" : "public-key");
    
    Serial.println("Headers configured, sending POST request...");
    
    // ファイル送信
    int httpCode = http.POST((uint8_t*)data, size);
    
    Serial.print("POST completed, HTTP code: ");
    Serial.println(httpCode);
    Serial.print("POST completed, HTTP code: ");
    Serial.println(httpCode);
    
    bool success = false;
    if (httpCode > 0) {
        Serial.printf("Response received: %d\n", httpCode);
        
        if (httpCode == 200 || httpCode == 201) {
            String response = http.getString();
            Serial.println("SUCCESS! Server response: " + response);
            
            M5.Display.println("\nSuccess!");
            M5.Display.print("Code: ");
            M5.Display.println(httpCode);
            success = true;
        } else {
            Serial.printf("Server returned error: %d\n", httpCode);
            String response = http.getString();
            Serial.println("Response body: " + response);
            
            M5.Display.println("\nServer Error!");
            M5.Display.print("Code: ");
            M5.Display.println(httpCode);
        }
    } else {
        String errorMsg = http.errorToString(httpCode);
        Serial.printf("HTTP Client error code: %d\n", httpCode);
        Serial.printf("Error message: %s\n", errorMsg.c_str());
        
        M5.Display.println("\nConnection Error!");
        M5.Display.print("Code: ");
        M5.Display.println(httpCode);
        M5.Display.println(errorMsg);
    }
    
    http.end();
    Serial.println("=== FileSender::sendFile() END ===\n");
    return success;
}

String FileSender::getContentType(FileType type) {
    switch(type) {
        case FileType::IMAGE:
            return "image/jpeg";
        case FileType::PUBLIC_KEY:
            return "application/x-pem-file";
        default:
            return "application/octet-stream";
    }
}

String FileSender::getFileExtension(FileType type) {
    switch(type) {
        case FileType::IMAGE:
            return ".jpg";
        case FileType::PUBLIC_KEY:
            return ".pub";
        default:
            return ".bin";
    }
}
