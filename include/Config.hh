#pragma once
#include <string>
#include <unordered_map>


class Config
{
public:
    Config(std::string const & fileName);

    int getInt(std::string const & key);
    std::string getStr(std::string const & key);

private:
    std::unordered_map<std::string, std::string> confMap;
};
