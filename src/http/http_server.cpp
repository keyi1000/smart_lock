#include "http/http_server.h"
#include <M5Unified.h>

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

void HTTPServer::handleRoot() {
    server->send(200, "text/plain", "M5Stack Server Ready");
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
