//
// Created by 谢子南 on 2023/7/18.
//
#include <data_queue.h>

int data_queue_t::push(const std::vector<char> &data) {
    locker.lock();
    if(queue.size()  > 1024) //包堆积了
    {
        locker.unlock();
        return 0;
    }
    queue.push(data);
    locker.unlock();
    return 1;
}

int data_queue_t::pop(std::vector<char> &data) {
    locker.lock();
    if(queue.empty())
    {
        locker.unlock();
        return 0;
    }
    data = queue.front();
    queue.pop();
    return 1;
}
