#include <cstring>
#include <stdexcept>
#include "HttpRequest.hh"

std::string urlDecode(std::string const & str){
    std::string ret;
    char ch;
    int i, ii, len = str.size();

    for (i=0; i < len; i++){
        if(str[i] != '%'){
            if(str[i] == '+')
                ret += ' ';
            else
                ret += str[i];
        }
        else {
            sscanf(str.substr(i + 1, 2).c_str(), "%x", &ii);
            ch = static_cast<char>(ii);
            ret += ch;
            i = i + 2;
        }
    }
    return ret;
}

namespace Http {

void HttpRequest::parseMethod() {
    while (isspace(*(inpData->ptr + inpData->cur)) && inpData->cur < inpData->size)
        inpData->cur++;

    if (!strncmp(inpData->ptr + inpData->cur, "GET", 3)) {
        inpData->cur += 3;
        method_ = Method::GET;
        state_ = ParseState::uri;
        return;
    } else if (!strncmp(inpData->ptr + inpData->cur, "HEAD", 4)) {
        inpData->cur += 4;
        method_ = Method::HEAD;
        state_ = ParseState::uri;
        return;
    }

    while (!isspace(*(inpData->ptr + inpData->cur)) && inpData->cur < inpData->size)
        inpData->cur++;
    state_ = ParseState::uri;
}

void HttpRequest::parseUri() {
    while (isspace(*(inpData->ptr + inpData->cur)) && inpData->cur < inpData->size)
        inpData->cur++;

    for (auto i = inpData->cur; i < inpData->size; ++i) {
        if (inpData->ptr[i] == ' ') {
            uri_ = URI(inpData->ptr + inpData->cur, i - inpData->cur);
            inpData->cur = i;
            uri_ = urlDecode(uri_);
            auto query = uri_.find("?");
            if (query != std::string::npos)
                uri_ = uri_.substr(0, query);
            state_ = ParseState::version;
            return;
        }
    }

    while (isspace(*(inpData->ptr + inpData->cur)) && inpData->cur < inpData->size)
        inpData->cur++;
    state_ = ParseState::version;
}

void HttpRequest::parseVersion() {
    while (isspace(*(inpData->ptr + inpData->cur)) && inpData->cur < inpData->size)
        inpData->cur++;

    if (!strncmp(inpData->ptr + inpData->cur, "HTTP/1.0", 8)) {
        inpData->cur += 8;
        version_ = Version::HTTP_1_0;
        state_ = ParseState::done;
        return;
    } else if (!strncmp(inpData->ptr + inpData->cur, "HTTP/1.1", 8)) {
        inpData->cur += 8;
        version_ = Version::HTTP_1_1;
        state_ = ParseState::done;
        return;
    }

    while (isspace(*(inpData->ptr + inpData->cur)) && inpData->cur < inpData->size)
        inpData->cur++;
    state_ = ParseState::done;
}

void HttpRequest::parse() {

    if (state_ == ParseState::done)
        return;

    if (inpData->cur == inpData->size)
        return;

    if (state_ == ParseState::method)
        parseMethod();

    if (state_ == ParseState::uri)
        parseUri();

    if (state_ == ParseState::version)
        parseVersion();
}

HttpRequest::ParseState HttpRequest::state() const noexcept {
    return state_;
}

void HttpRequest::write(char const *data, uint32_t len) {
    write_to_data(outData, data, len);
}

void HttpRequest::write(std::string const & data) {
    write_to_data(outData, data.data(), data.size());
}

const Method &HttpRequest::method() const noexcept {
    return method_;
}

const Version &HttpRequest::version() const noexcept {
    return version_;
}

const URI &HttpRequest::uri() const noexcept {
    return uri_;
}

uint32_t HttpRequest::id() const noexcept {
    return id_;
}

} //namespace Http