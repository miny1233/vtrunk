#include <iostream>
#include <kcp.h>
#include <queue>
#include <mutex>
#include <format>
#include <nlohmann/json.hpp>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <set>
#include <tunnel.h>
#include <load_balance.h>
#include <Log.h>
#include <fstream>

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

#define DEFAULT_CONFIG

int main(int argc,char* argv[]) {
    LOG("Loading config");
    nlohmann::json v_conf;
    if(argc != 2) {
        LOG("Use default config");
        v_conf["bind_ip"] = "127.0.0.1";
        v_conf["bind_port"] = 13777;
        auto &_tunnel = v_conf["tunnel"]["default_tunnel"];
        _tunnel["remote_ip"] = "127.0.0.1";
        _tunnel["remote_port"] = 13778;
        _tunnel["bind_ip"] = "0.0.0.0";
        _tunnel["bind_port"] = 0;
    }else {
        std::string conf_path = argv[1];
        std::fstream conf_raw_fd(conf_path,std::ios::binary | std::ios::in);
        conf_raw_fd.seekg(0, std::ios::end);    // go to the end
        auto length = conf_raw_fd.tellg();           // report location (this is the length)
        conf_raw_fd.seekg(0, std::ios::beg);
        std::vector<char> conf_buf(length);
        conf_raw_fd.read(&conf_buf[0],length);
        v_conf = nlohmann::json::parse(&conf_buf[0]);
    }
    //LOG("config is :");
    //for(char ch : to_string(v_conf))
    //{
    //    std::cout<<ch;
    //    if(ch == '{' | ch == '}' | ch == ',')std::cout<<std::endl;
    //}
    LOG("init vtrunk!");
    //绑定本地入口套接字 TCP
    auto in_sock = socket(AF_INET,SOCK_STREAM,0);
    assert(in_sock != -1);
    sockaddr_in local{};
    local.sin_family = AF_INET;
    local.sin_port = htons(v_conf["bind_port"].get<uint16_t>()); //大小端转换
    local.sin_addr.s_addr = htonl(ip_to_b(v_conf["bind_ip"].get<std::string>()));
    assert(-1 != bind(in_sock,(sockaddr*)&local,sizeof(local)));

    //负载均衡(Ai)
    load_balance loadBalance = load_balance();

    //创建隧道
    for(auto [key_name,_tunnel] : v_conf["tunnel"].items()) {
        LOG("Create tunnel {} \nremote ip: {} remote port: {}\n"
            "bind ip: {} , bind port: {}",
           key_name,
           _tunnel["remote_ip"].get<std::string>(),_tunnel["remote_port"].get<uint16_t>(),
                   _tunnel["bind_ip"].get<std::string>(),_tunnel["bind_port"].get<uint16_t>());
        auto t_fd = socket(AF_INET,SOCK_DGRAM,0);
        assert(-1 != t_fd);
        sockaddr_in re_temp{};
        re_temp.sin_family = AF_INET;
        re_temp.sin_addr.s_addr = htonl(ip_to_b(_tunnel["bind_ip"].get<std::string>())),
        re_temp.sin_port = htons(_tunnel["bind_port"].get<uint16_t>());
        assert(-1 != bind(t_fd,(sockaddr*)&re_temp,sizeof re_temp));

        re_temp.sin_family = AF_INET,
        re_temp.sin_addr.s_addr = htonl(ip_to_b(_tunnel["remote_ip"].get<std::string>())),
        re_temp.sin_port = htons(_tunnel["remote_port"].get<uint16_t>());
        tunnel t = tunnel(t_fd, *reinterpret_cast<sockaddr*>(&re_temp));
        loadBalance.add_tunnel(t);
    }

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
    forward_back.join(); //一个寄了程序直接寄
    // forward_to.join(); // send要收到数据才会寄，所以只看back
    return 0;
}
