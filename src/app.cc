#include <HttpServer.hh>

extern "C" {

int app_init() {
    Http::HttpServer::init(8080);
}

}

