#include <sstream>
#include "HttpResponse.hh"

void Http::HttpResponse::addHeader(const std::string &key, const std::string &value) {
    headerMap.insert(std::make_pair(key, value));
}

std::string Http::HttpResponse::raw() {
    std::stringstream ss;
    if (version_ == Version::HTTP_1_0)
        ss << "HTTP/1.1 ";
    else
        ss << "HTTP/1.0 ";

    ss << code_ << " ";
    if (code_ == 200)
        ss << "OK\r\n";
    else if (code_ == 404)
        ss << "Not Found\r\n";
    else if (code_ == 405)
        ss << "Method Not Allowed\r\n";
    else if (code_ == 403)
        ss << "Forbidden\r\n";

    for (auto item: headerMap)
        ss << item.first << ": " << item.second << "\r\n";

    ss << "\r\n";

    return ss.str();
}

void Http::HttpResponse::setContentType(std::string const & file) {
    if (file.find(".css") != std::string::npos)
        addHeader("Content-Type", "text/css");
    else if (file.find(".html") != std::string::npos)
        addHeader("Content-Type", "text/html");
    else if (file.find(".js") != std::string::npos)
        addHeader("Content-Type", "application/javascript");
    else if (file.find(".gif") != std::string::npos)
        addHeader("Content-Type", "image/gif");
    else if (file.find(".jpeg") != std::string::npos)
        addHeader("Content-Type", "image/jpeg");
    else if (file.find(".jpg") != std::string::npos)
        addHeader("Content-Type", "image/jpeg");
    else if (file.find(".png") != std::string::npos)
        addHeader("Content-Type", "image/png");
    else if (file.find(".swf") != std::string::npos)
        addHeader("Content-Type", "application/x-shockwave-flash");
}

void Http::HttpResponse::code(uint32_t code) {
    code_ = code;
}
