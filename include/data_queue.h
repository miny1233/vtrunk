//
// Created by 谢子南 on 2023/7/18.
//

#ifndef VTRUNK_DATA_QUEUE_H
#define VTRUNK_DATA_QUEUE_H

#include <queue>
#include <mutex>
/*
 *  -1 为失败 0为繁忙或空 1为完成
 */
class data_queue_t
{
public:
    data_queue_t() = default;
    int push(const std::vector<char>& data);
    int pop(std::vector<char>& data);
private:
    std::mutex locker;
    std::queue<std::vector<char>> queue;
};

#endif //VTRUNK_DATA_QUEUE_H
