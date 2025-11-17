#include "http/http_server.h"
#include <M5Unified.h>
#include <LittleFS.h>

WebServer serverInstance(80);
HTTPServer* currentInstance = nullptr;

void HTTPServer::begin() {
    currentInstance = this;
    server = &serverInstance;
    
    // ルート設定
    server->on("/", HTTP_GET, [](){ currentInstance->handleRoot(); });
    server->on("/", HTTP_POST, [](){ currentInstance->handlePost(); });
    server->onNotFound([](){ currentInstance->handleNotFound(); });
    
    server->begin();
    Serial.println("HTTP server started");
}

void HTTPServer::handleClient() {
    server->handleClient();
}

void HTTPServer::setTargetServer(const char* url, const char* username, const char* password) {
    fileSender.setTargetServer(url);
    fileSender.setAuth(username, password);
}

void HTTPServer::handleRoot() {
    // GETリクエストを受けたら画像を別サーバーに送信
    Serial.println("\n>>> GET request received <<<");
    
    M5.Display.clear();
    M5.Display.setCursor(0, 0);
    M5.Display.println("GET request");
    M5.Display.println("Sending image...");
    
    sendImageToServer();
    
    server->send(200, "text/plain", "Image sent");
    Serial.println(">>> GET request completed <<<\n");
}

void HTTPServer::sendImageToServer() {
    Serial.println("--- Preparing to send image ---");
    
    // LittleFSからshibata.jpgを読み込み
    File file = LittleFS.open("/shibata.jpg", "r");
    if (!file) {
        Serial.println("✗ Failed to open shibata.jpg");
        M5.Display.println("File Error!");
        return;
    }
    
    size_t fileSize = file.size();
    Serial.print("File size: ");
    Serial.print(fileSize);
    Serial.println(" bytes");
    
    // ファイルをメモリに読み込み
    uint8_t* imageData = (uint8_t*)malloc(fileSize);
    if (!imageData) {
        Serial.println("✗ Memory allocation failed");
        M5.Display.println("Memory Error!");
        file.close();
        return;
    }
    
    file.read(imageData, fileSize);
    file.close();
    
    Serial.println("Calling fileSender.sendFile()...");
    
    // ファイル送信
    bool success = fileSender.sendFile(
        imageData, 
        fileSize, 
        FileType::IMAGE, 
        "shibata.jpg"
    );
    
    // メモリ解放
    free(imageData);
    
    if (success) {
        M5.Display.println("Sent!");
        Serial.println("✓ Image sent successfully");
    } else {
        M5.Display.println("Failed!");
        Serial.println("✗ Failed to send image");
    }
    Serial.println("--- Send image completed ---\n");
}

void HTTPServer::handlePost() {
    // POSTデータを取得
    String body = server->arg("plain");
    
    Serial.println("POST received:");
    Serial.println(body);
    
    // ディスプレイに表示
    M5.Display.clear();
    M5.Display.setCursor(0, 0);
    M5.Display.println("POST Received:");
    M5.Display.println(body);
    
    // レスポンスを返す
    server->send(200, "application/json", "{\"status\":\"ok\"}");
}

void HTTPServer::handleNotFound() {
    server->send(404, "text/plain", "Not Found");
}
