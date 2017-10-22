#include <cstdint>
#include <ctime>
#include <HttpServer.hh>
#include <HttpResponse.hh>
#include <fstream>
#include <sys/stat.h>

long fileSize(std::string const & filename)
{
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

namespace Http {

std::string HttpServer::documentRoot = "/home/algys/sources/http-server/http-test-suite";

void HttpServer::onRead(void *udata) {
    if (!udata)
        return;

    auto request = static_cast<HttpRequest *>(udata);

    request->parse();

    if (request->state() == HttpRequest::ParseState::done) {
        set_need_read(request->id(), 0);
        instance().handler(*request);
    }
}

void HttpServer::onAccept(uint32_t id, data_t *inp_data, data_t *out_data, void **udata) {
    auto request = new HttpRequest(id, inp_data, out_data);
    *udata = static_cast<void *>(request);
}

void HttpServer::handler(HttpRequest &request) {
    std::string file;
    bool isIndex = false;
    if (request.uri()[request.uri().size()-1] == '/') {
        file = request.uri() + "index.html";
        isIndex = true;
    }
    else
        file = request.uri();

    HttpResponse response(Version::HTTP_1_1, 200);

    auto time = std::time(nullptr);
    auto timeStr = std::string(asctime(std::localtime(&time)));
    timeStr = timeStr.substr(0, timeStr.size() - 1);
    response.addHeader("Date", timeStr);
    response.addHeader("Server", "Algys");

    if (request.method() == Method::invalid) {
        response.code(405);
        request.write(response.raw());
    }
    else if (request.method() == Method::GET){
        std::fstream fs;
        fs.open(documentRoot + file);

        if (!fs.is_open()) {
            response.code(isIndex ? 403: 404);
            request.write(response.raw());
        } else {
            response.code(200);
            response.setContentType(file);
            response.addHeader("Content-Length", std::to_string(fileSize(documentRoot + file)));

            request.write(response.raw());
            static char buf[1024];
            static uint32_t len;
            while ((len = fs.readsome(buf, 1024)))
                request.write(buf, len);
        }
    }
    else if (request.method() == Method::HEAD) {
        std::fstream fs;
        fs.open(documentRoot + file);

        if (!fs.is_open()) {
            response.code(isIndex ? 403: 404);
            request.write(response.raw());
        } else {
            response.code(200);
            response.setContentType(file);
            response.addHeader("Content-Length", std::to_string(fileSize(documentRoot + file)));
            request.write(response.raw());
        }
    }

    destroy(request.id());

    return;
}

HttpServer &HttpServer::instance() {
    static HttpServer instance;
    return instance;
}

void HttpServer::destroy(uint32_t id) {
    need_destroy(id);
}

void HttpServer::init(uint32_t port) {
    //  HttpServer::port_ = port;
    ::init(&HttpServer::onAccept, &HttpServer::onRead, &HttpServer::onDestroy);
    new HttpServer();
}

void HttpServer::onDestroy(void *udata) {
    delete static_cast<HttpRequest*>(udata);
}

} //namespace Http