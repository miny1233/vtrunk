#include <iostream>
#include <kcp.h>
#include <queue>
#include <atomic>
#include <mutex>
#include <memory.h>
#include <format>
#include <nlohmann/json.hpp>

int main() {

    nlohmann::json j = R"({
        "pi": 3.14
    })"_json;
    int pi = j["pi"];
    std::cout<<pi;
    std::mutex locker1,locker2;
    std::queue<std::vector<char>> upload1,upload2;
    kcp_io netio1{
        .send = [&upload1,&locker1](const char* msg,int len) -> auto {
            locker1.lock();
            std::vector<char> data(len);
            memcpy(&data[0],msg,len);
            upload1.push(data);
            locker1.unlock();
            return len;
        },
        .recv = [&upload2,&locker2](char* buf,int buf_size) -> int {
            locker2.lock();
            if(upload2.empty()){
                locker2.unlock();
                return -1;
            }
            auto data = upload2.front();
            upload2.pop();
            locker2.unlock();
            memcpy(buf,&data[0],data.size());
            return data.size();
        }
    };
    kcp_io netio2{
            .send = [&upload2,&locker2](const char* msg,int len) -> auto {
                locker2.lock();
                std::vector<char> data(len);
                memcpy(&data[0],msg,len);
                upload2.push(data);
                locker2.unlock();
                return len;
            },
            .recv = [&upload1,&locker1](char* buf,int buf_size) -> int {
                locker1.lock();
                if(upload1.empty()){
                    locker1.unlock();
                    return -1;
                }
                auto data = upload1.front();
                upload1.pop();
                locker1.unlock();
                memcpy(buf,&data[0],data.size());
                return data.size();
            }
    };

    kcp k1(netio1,1),k2(netio2,1);
    std::vector<char> buf(1024 * 1024);
    std::string msg = "hi my name is k1\n";
    std::cout<<k1.send(msg.c_str(),msg.size())<<std::endl;
    msg = "how are you";
    std::cout<<k1.send(msg.c_str(),msg.size())<<std::endl;
    int len = 0;
    std::ios_base::sync_with_stdio();
    while(true) {
        len = k2.recv(&buf[0], buf.size());
        std::cout << std::format("Recv Msg Len is : {}\n", len);
        for(int i=0;i<len;i++)
        {
            std::cout<<buf[i];
        }
        std::cout<<std::flush;
    }
}
