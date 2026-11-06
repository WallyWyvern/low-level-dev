//
// Created by walte on 01/11/2026.
//

#ifndef ASIO_CHAT_BAREBONES_USER_H
#define ASIO_CHAT_BAREBONES_USER_H

#endif //ASIO_CHAT_BAREBONES_USER_H
struct user
{
    std::string password;
    bool isAdmin = false;
    bool isBanned = false;

    SessionPtr currentSession;
};