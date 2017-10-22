#pragma once

#include <cstdint>
#include <unordered_map>
#include "HttpRequest.hh"

namespace Http {

class HttpResponse {
public:
    using HeaderMap = std::unordered_map<std::string, std::string>;

public:
    HttpResponse(Version version, uint32_t code):
        version_ { version },
        code_ { code }
    { }

    void code(uint32_t code);

    void addHeader(std::string const & key, std::string const & value);
    void setContentType(std::string const & file);

    std::string raw();

private:
    Http::Version version_;
    uint32_t code_;

    HeaderMap headerMap;
};

} //namespace Http