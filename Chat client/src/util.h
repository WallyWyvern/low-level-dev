//
// Created by walte on 01/11/2026.
//

#ifndef ASIO_CHAT_BAREBONES_UTIL_H
#define ASIO_CHAT_BAREBONES_UTIL_H

#endif //ASIO_CHAT_BAREBONES_UTIL_H
#include <vector>
#include <string>
#pragma once

struct util
{
    // split function found here: https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
   static std::vector<std::string> split(std::string s, const std::string& delimiter) {
        std::vector<std::string> tokens;
        size_t pos = 0;
        std::string token;
        while ((pos = s.find(delimiter)) != std::string::npos) {
            token = s.substr(0, pos);
            tokens.push_back(token);
            s.erase(0, pos + delimiter.length());
        }
        tokens.push_back(s);

        return tokens;
    }
};