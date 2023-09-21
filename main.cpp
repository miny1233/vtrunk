#pragma GCC optimize("O2")

#include <iostream>
#include <kcp.h>
#include <queue>
#include <atomic>
#include <mutex>
#include <memory.h>
#include <format>
#include <data_queue.h>
#include <nlohmann/json.hpp>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define UNIX_SOCKER

int main() {
    std::string local_ip = "127.0.0.1"; //本地接口
    uint16_t port = 13888;
    int in_sock;
#ifdef UNIX_SOCKER
    in_sock = socket(AF_UNIX,SOCK_DGRAM,IPPROTO_UDP);
    sockaddr_in local{};
    local.sin_family = AF_UNIX;
    local.sin_port = port;
    assert(inet_aton(local_ip.c_str(),&local.sin_addr));
    assert(bind(in_sock,(sockaddr*)&local,sizeof(local)));
#endif
    
}
