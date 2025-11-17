#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <WebServer.h>
#include "file/file_sender.h"

class HTTPServer {
public:
    void begin();
    void handleClient();
    void setTargetServer(const char* url, const char* username, const char* password);
    
private:
    WebServer* server;
    FileSender fileSender;
    
    void handleRoot();
    void handlePost();
    void handleNotFound();
    void sendImageToServer();
};

#endif
