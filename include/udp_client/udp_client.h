//
// Created by noahyzhang on 2021/10/2.
//

#ifndef UDP_CLIENT_UDP_CLIENT_H
#define UDP_CLIENT_UDP_CLIENT_H

#include <string>

#include <netinet/in.h>

namespace UdpClient {

    typedef struct {
        int code_;
        std::string body_;
    } Response;

    class Client {
    public:
        Client() noexcept;
        ~Client() noexcept;
        Client(const Client& other) = delete;
        Client& operator=(const Client& other) = delete;
        Client(Client&& other) noexcept;
        Client& operator=(Client&& other) noexcept;

    public:
        bool InitClient(const char* host, int port);
        bool AsyncSend(const std::string& msg);
        std::unique_ptr<Response> SyncSend(const std::string& msg, unsigned int timeout_ms);

    private:
        int sock_fd_;
        struct sockaddr_in remote_addr_;
    };
}

#endif //UDP_CLIENT_UDP_CLIENT_H
