#include <iostream>
#include <kcp.h>
#include <queue>
#include <atomic>
#include <mutex>
#include <memory.h>
#include <format>
#include <data_queue.h>
#include <nlohmann/json.hpp>

int main() {
    std::vector<char> msg(15);
    strcpy(&msg[0],"hello world");
    auto q = data_queue_t();
    q.push(msg);
    std::vector<char> buf;
    q.pop(buf);
    for(auto ch:buf)
    {
        std::cout<<ch;
    }
    return 1;
}
