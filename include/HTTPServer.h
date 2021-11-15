#ifndef BELL_HTTP_SERVER_H
#define BELL_HTTP_SERVER_H

#include <functional>
#include <map>
#include <optional>
#include <memory>
#include <regex>
#include <optional>
#include <set>
#include <iostream>
#include <queue>
#include <stdio.h>
#include <unistd.h>
#include <sstream>
#include <BellLogger.h>
#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string>
#include <netdb.h>
#include <fcntl.h>

#ifndef SOCK_NONBLOCK
#define SOCK_NONBLOCK O_NONBLOCK
#endif

namespace bell {
enum class RequestType {
    GET,
    POST
};

struct HTTPRequest {
    std::map<std::string, std::string> urlParams;
    std::map<std::string, std::string> queryParams;
    std::string body;
    int handlerId;
    int connection;
};

struct HTTPResponse {
    int connectionFd;
    int status;
    std::string body;
    std::string contentType;
};

typedef std::function<void(HTTPRequest&)> httpHandler;
struct HTTPRoute {
    RequestType requestType;
    httpHandler handler;
};

struct HTTPConnection {
    std::vector<uint8_t> buffer;
    std::string currentLine = "";
    int contentLength = 0;
    bool isReadingBody = false;
    std::string httpMethod;
};

class HTTPServer {
private:
    std::regex routerPattern = std::regex(":([^\\/]+)?");
    fd_set master;
    fd_set readFds;
    int pipeFd[2];
    
    std::queue<HTTPResponse> responseQueue;
    fd_set activeFdSet, readFdSet;
    bool isClosed = true;
    bool writingResponse = false;
    std::map<std::string, std::vector<HTTPRoute>> routes;
    std::map<int, HTTPConnection> connections;
    void writeResponse(const HTTPResponse&);
    void findAndHandleRoute(std::string&, std::string&, int connectionFd);
    std::vector<std::string> splitUrl(const std::string& url, char delimiter);
    void readFromClient(int clientFd);
    std::map<std::string, std::string> parseQueryString(const std::string &queryString);
    unsigned char h2int(char c);
    std::string urlDecode(std::string str);

public:
    HTTPServer(int serverPort);
    
    int serverPort;
    void registerHandler(RequestType requestType, const std::string&, httpHandler);
    void respond(const HTTPResponse&);
    void closeConnection(int connection);
    void listen();
};
}
#endif


