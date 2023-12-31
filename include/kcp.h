//
// Created by 谢子南 on 2023/7/2.
//

#ifndef VTRUNK_KCP_H
#define VTRUNK_KCP_H

#include <iostream>
#include "../kcp/ikcp.h"
#include <thread>
#include <sys/time.h>
#include <vector>
#include <functional>
#include <mutex>
#include <condition_variable>

#define BUFFER_SIZE (32 * 1024)

//kcp与底层通信
struct kcp_io{
    std::function<int(const char* package, int package_size)> send;
    std::function<int(char* buf,int buf_size)> recv;   //阻塞型函数
};

class kcp {
public:
    /* get system time */
   static inline void itimeofday(long *sec, long *usec) {
        timeval time{};
        gettimeofday(&time, nullptr);
        if (sec) *sec = time.tv_sec;
        if (usec) *usec = time.tv_usec;
    }

/* get clock in millisecond 64 */
    static inline IINT64 iclock64() {
        long s, u;
        IINT64 value;
        itimeofday(&s, &u);
        value = ((IINT64) s) * 1000 + (u / 1000);
        return value;
    }

    static inline IUINT32 iclock() {
        return (IUINT32) (iclock64() & 0xfffffffful);
    }
public:
    kcp(kcp_io& kcpIo,u_int32_t id,int mode = 0);
    ssize_t send(const void* msg,size_t len);
    ssize_t recv(void* buf,size_t buf_size);
private:
    [[noreturn]] void update();
    [[noreturn]] void recv_from_io();
    kcp_io& io;
    ikcpcb* ikcp;
    std::thread update_task,recv_task;
    std::mutex recv_lock; // 全局互斥锁
    std::mutex kcp_lock;  // kcp锁 kcp是多线程不安全的 这里最好使用自旋锁
    std::condition_variable recv_ok; // 收到数据 可以尝试读取
    std::unique_lock<std::mutex> recv_l;// 用来通知可能有消息可用
};


#endif //VTRUNK_KCP_H
