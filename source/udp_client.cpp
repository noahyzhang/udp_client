//
// Created by noahyzhang on 2021/10/2.
//

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "udp_client/udp_client.h"

namespace UdpClient {

    Client::Client() noexcept {
        sock_fd_ = -1;
        memset(&remote_addr_, 0, sizeof(remote_addr_));
    }

    Client::~Client() noexcept {
        close(sock_fd_);
    }

    Client::Client(Client&& other) noexcept {
        sock_fd_ = other.sock_fd_;
        remote_addr_ = other.remote_addr_;
        other.sock_fd_ = -1;
        memset(&other.remote_addr_, 0, sizeof(other.remote_addr_));
    }

    Client& Client::operator=(Client&& other)  noexcept {
        sock_fd_ = other.sock_fd_;
        remote_addr_ = other.remote_addr_;
        other.sock_fd_ = -1;
        memset(&other.remote_addr_, 0, sizeof(other.remote_addr_));
        return *this;
    }

    bool Client::InitClient(const char *host, int port) {
        sock_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock_fd_ < 0) {
            return false;
        }
        remote_addr_.sin_family = AF_INET;
        remote_addr_.sin_addr.s_addr = inet_addr(host);
        remote_addr_.sin_port = htons(port);
        return true;
    }

    bool Client::AsyncSend(const std::string &msg) {
        auto remote_addr = reinterpret_cast<struct sockaddr*>(&remote_addr_);
        socklen_t remote_addr_len = sizeof(*remote_addr);
        int send_size = sendto(sock_fd_, msg.c_str(), msg.size(), 0, remote_addr, remote_addr_len);
        if (send_size == msg.size()) {
            return true;
        }
        return false;
    }

    std::unique_ptr<Response> Client::SyncSend(const std::string &msg, unsigned int timeout_ms) {
        auto resp = std::unique_ptr<Response>(new Response);
        if (!AsyncSend(msg)) {
            resp->code_ = -1;
            resp->body_ = std::string("send data failed");
            return resp;
        }

        struct timeval tv{};
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
        int ret = setsockopt(sock_fd_, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
        if (ret != 0) {
            resp->code_ = ret;
            resp->body_ = std::string("setsockopt set SO_RCVTIMEO error");
        }
        char recv_buf[1472] = {0};
        int recv_len = recvfrom(sock_fd_, recv_buf, 1472, 0, nullptr, nullptr);
        if (recv_len <= 0) {
            resp->code_ = -2;
            resp->body_ = std::string("recv data failed");
            return resp;
        }
        recv_buf[recv_len] = 0;
        resp->code_ = 0;
        resp->body_ = recv_buf;
        return resp;
    }
}