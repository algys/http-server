#include <HttpServer.hh>
#include <Config.hh>

extern "C" {

int app_init(int argc, char ** argv) {
    if (argc < 2)
        throw std::runtime_error("Too few args");

    auto conf = Config(argv[1]);

    Http::HttpServer::init((uint32_t) conf.getInt("listen"), conf.getStr("document_root"));
}

}

