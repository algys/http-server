#pragma once
#include <string>
#include <functional>

extern "C" {
    #include "core.h"
}

namespace Http {

enum class Method {
    GET = 0,
    HEAD = 1,
    invalid,
};

enum class Version {
    HTTP_1_0 = 0,
    HTTP_1_1 = 1,
    invalid,
};

using URI = std::string;


class HttpRequest {
public:
    enum class ParseState {
        method = 0,
        uri = 1,
        version = 2,
        done,
    };

public:
    HttpRequest(HttpRequest const &) = delete;
    HttpRequest &operator=(HttpRequest const &) = delete;

    HttpRequest(uint32_t id, data_t *inpData, data_t *outData) :
        id_{id},
        inpData{inpData},
        outData{outData}
    { }

    uint32_t id() const noexcept;

    Method const &method() const noexcept;
    Version const &version() const noexcept;
    URI const &uri() const noexcept;

    void parse();
    ParseState state() const noexcept;

    void write(char const *data, uint32_t len);
    void write(std::string const & data);

private:
    data_t *inpData;
    data_t *outData;

    uint32_t id_;
    Method method_{Method::invalid};
    Version version_{Version::invalid};
    URI uri_{};

    ParseState state_{ParseState::method};

    void parseVersion();

    void parseUri();

    void parseMethod();
};

} //namespace Http