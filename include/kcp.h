//
// Created by 谢子南 on 2023/7/2.
//

#ifndef VTRUNK_KCP_H
#define VTRUNK_KCP_H

#define BUFFER_SIZE (512 * 1024) //缓冲区 512KB

#include <iostream>
#include "../kcp/ikcp.h"
#include <thread>
#include <sys/time.h>
#include <vector>
#include <functional>
#include <mutex>

namespace iKcp {

}

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
        gettimeofday(&time, NULL);
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
    int send(const char* msg,int len);
    int recv(char* buf,int buf_size);
private:
    [[noreturn]] void update();
    [[noreturn]] void recv_from_io();
    kcp_io& io;
    ikcpcb* ikcp;
    std::thread update_task,recv_task;
    std::mutex recv_lock; // 全局互斥锁
    std::condition_variable recv_ok; // 收到数据 可以尝试读取
};


#endif //VTRUNK_KCP_H
