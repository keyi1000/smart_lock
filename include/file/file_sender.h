#ifndef FILE_SENDER_H
#define FILE_SENDER_H

#include <HTTPClient.h>
#include <Arduino.h>

enum class FileType {
    IMAGE,
    PUBLIC_KEY,
    UNKNOWN
};

class FileSender {
public:
    void setTargetServer(const char* url);
    void setAuth(const char* username, const char* password);
    bool sendFile(const uint8_t* data, size_t size, FileType type, const char* filename);
    
private:
    String targetServerUrl;
    String authUsername;
    String authPassword;
    String getContentType(FileType type);
    String getFileExtension(FileType type);
};

#endif
