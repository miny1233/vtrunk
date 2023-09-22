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

int main() {
    std::string local_ip = "127.0.0.1"; //本地接口
    uint16_t port = 13777;
    int in_sock;
    //绑定本地入口套接字
    in_sock = socket(AF_UNIX,SOCK_DGRAM,0);
    assert(in_sock != -1);
    sockaddr_in local{};
    local.sin_family = AF_UNIX;
    local.sin_port = port;
    assert(inet_aton(local_ip.c_str(),&local.sin_addr));
    assert(bind(in_sock,(sockaddr*)&local,sizeof(local)));
    //创建隧道
    sockaddr a{};
    tunnel t = tunnel(1,a);

    load_balance loadBalance = load_balance();
    loadBalance.add_tunnel(t);
    //启动
}
