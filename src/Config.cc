#include <fstream>
#include <sstream>
#include "Config.hh"

Config::Config(const std::string &fileName) {
    std::fstream fs;
    fs.open(fileName);

    if (!fs.is_open())
        throw std::runtime_error("Can't open conf file");

    std::string line;
    while (std::getline(fs, line)) {
        std::stringstream lineStream(line);
        std::string key, value;
        lineStream >> key >> value;

        if (key.size() && value.size())
            confMap[key] = value;
    }
}

int Config::getInt(std::string const &key) {
    auto val = getStr(key);

    int res;
    std::stringstream ss(val);
    ss >> res;

    return res;
}

std::string Config::getStr(std::string const &key) {
    auto val = confMap.find(key);
    if (val == confMap.end())
        throw std::logic_error("Key not exist");

    return val->second;
}


