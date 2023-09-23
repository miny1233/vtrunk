#include <iostream>
#include <kcp.h>
#include <queue>
#include <atomic>
#include <mutex>
#include <memory.h>
#include <format>
#include <nlohmann/json.hpp>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <set>
#include <tunnel.h>
#include <load_balance.h>
#include <Log.h>

#define BUF_SIZE (512 * 1024)

uint32_t ip_to_b(const std::string& ip)
{
    std::stringstream stringstream(ip);
    std::string field;
    uint32_t ret = 0;
    while(getline(stringstream,field,'.'))
    {
        std::stringstream to_uint(field);
        uint32_t _field;
        to_uint>>_field;
        ret <<= 8;
        ret |= _field;
    }
    return ret;
}

int main() {
    LOG("init vtrunk!");
    //std::string local_ip = "127.0.0.1"; //本地接口
    uint32_t port = 1377;
    int in_sock,ret;
    //绑定本地入口套接字 TCP
    in_sock = socket(AF_INET,SOCK_STREAM,0);
    assert(in_sock != -1);
    sockaddr_in local{};
    local.sin_family = AF_INET;
    local.sin_port = htons(port); //大小端转换
    local.sin_addr.s_addr = htonl(ip_to_b("127.0.0.1")); //127.0.0.1
    assert(-1 != bind(in_sock,(sockaddr*)&local,sizeof(local)));
    //创建隧道
    tunnel t = tunnel(1,sockaddr{});

    load_balance loadBalance = load_balance();
    loadBalance.add_tunnel(t);

    assert(!listen(in_sock,1));

    sockaddr local_user{};
    socklen_t local_user_len{};

    LOG("Wait connecting for user");
    in_sock = accept(in_sock,&local_user,&local_user_len);
    LOG("Connected");
    LOG("Running Now!");

    kcp_io kcpIo {
        .send = [&](const char* package, int package_size)-> ssize_t {
            return loadBalance.send_next(package,package_size);
        },
        .recv = [&](char* buf,int buf_size) -> ssize_t {
            return loadBalance.recv_next(buf,buf_size);
        }
    };

    kcp _kcp = kcp(kcpIo,1233);
    //启动
    std::thread forward_to = std::thread([&]() -> void {
        char *buf = new char[BUF_SIZE];
        while (true) {
            auto pg_size = recv(in_sock, buf, BUF_SIZE, 0);
            pg_size = _kcp.send(buf, pg_size);
            if (pg_size < 0) [[unlikely]] {
                std::cerr << std::format("Error: in forward_to KCP error ret = {}\n", pg_size);
                exit(-1);
            }
        }
    });

    std::thread forward_back = std::thread ([&]() -> void {
        char *buf = new char[BUF_SIZE];
        while (true) {
            socklen_t len;
            sockaddr recv_from{};
            auto pg_size = _kcp.recv(buf,BUF_SIZE);
            if (pg_size < 0) [[unlikely]]
            {
                std::cerr << std::format("Error: in forward_back KCP error ret = {}\n", pg_size);
                exit(-1);
            }
            send(in_sock,buf,pg_size,0);
        }
    });
    forward_back.join();
    forward_to.join();
    return 0;
}
