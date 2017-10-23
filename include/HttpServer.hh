#pragma once
#include <unordered_map>
#include "HttpRequest.hh"

extern "C" {
    #include "core.h"
}

namespace Http {

class HttpServer {
public:
    static void init(uint32_t port, std::string const & dRoot);
    static HttpServer &instance();

    HttpServer(HttpServer const &) = delete;
    HttpServer(HttpServer &&) = delete;
    HttpServer &operator=(HttpServer const &) = delete;
    HttpServer &operator=(HttpServer &&) = delete;

    void destroy(uint32_t id);

    static uint32_t port;
    static std::string documentRoot;

private:
    HttpServer() = default;

    static void onRead(void *udata);
    static void onAccept(uint32_t id, data_t *inp_data, data_t *out_data, void **udata);
    static void onDestroy(void *udata);

    virtual void handler(HttpRequest &request);
};

} //namespace Http