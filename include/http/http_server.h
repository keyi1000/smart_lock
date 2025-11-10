#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <WebServer.h>

class HTTPServer {
public:
    void begin();
    void handleClient();
    
private:
    WebServer* server;
    
    void handleRoot();
    void handlePost();
    void handleNotFound();
};

#endif
