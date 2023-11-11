
#include "kcp.h"

kcp::kcp(kcp_io &kcpIo,u_int32_t id,int mode):io(kcpIo),recv_l (recv_lock){
    ikcp = ikcp_create(id,this);
    // 输出回调
    ikcp->output = [](const char *buf, int len, ikcpcb *kcp, void *user) -> int{
        auto* kcp_model = reinterpret_cast<decltype(this)>(user);
        return kcp_model->io.send(buf,len);
    };

    switch (mode) {
        case 0:ikcp_nodelay(ikcp,0, 10, 0, 0);
            break;
        case 1:ikcp_nodelay(ikcp, 1, 10, 2, 1);
            break;
        default:
            throw std::exception();
    }


    ikcp_wndsize(ikcp,1024,2048);
    ikcp_setmtu(ikcp,9000);

    ikcp->interval = 0;
    //ikcp->rx_minrto = 5;

    update_task = std::thread(&kcp::update,this);
    recv_task = std::thread(&kcp::recv_from_io,this);
    if(!update_task.joinable() || !recv_task.joinable())
        throw std::exception();
}

[[noreturn]] void kcp::update() {
    while (true) {
        kcp_lock.lock();
        ikcp_update(ikcp, kcp::iclock());
        kcp_lock.unlock();
        //auto next_flush = ikcp_check(ikcp, kcp::iclock());
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        //std::this_thread::yield();
    }
}

[[noreturn]] void kcp::recv_from_io() {
    char* buf = new char[BUFFER_SIZE];
    int len;
    while(true) {
        len = io.recv(buf,BUFFER_SIZE);

        if(len == -1)
        {
            std::this_thread::yield();
            //std::this_thread::sleep_for(std::chrono::microseconds(1));
            continue;  // -1是没有接收到 0为接收到长度是0的包
        }

        kcp_lock.lock();
        ikcp_input(ikcp,buf,len);
        ikcp_update(ikcp, kcp::iclock());
        kcp_lock.unlock();

        // 收到了数据 尝试能否读
        std::unique_lock<std::mutex> get_msg(recv_lock);
        recv_ok.notify_all();
    }
}

ssize_t kcp::send(const void* msg, size_t len) {
    assert((int)len >= 0);
    for(;;){

        auto waitsnd = ikcp_waitsnd(ikcp);

        //std::cout<<"wait snd:" << waitsnd << "\n";
        if(waitsnd < 5000)
            break;

        //std::this_thread::yield();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        //while(ikcp_waitsnd(ikcp));
    }

    kcp_lock.lock();
    int ret = ikcp_send(ikcp,reinterpret_cast<const char *>(msg),(int)len);
    ikcp_update(ikcp, kcp::iclock());
    kcp_lock.unlock();

    return ret;
}

ssize_t kcp::recv(void* buf, size_t buf_size) {
    read_date:

    kcp_lock.lock();
    int len = ikcp_recv(ikcp,reinterpret_cast<char*>(buf), (int)buf_size);
    kcp_lock.unlock();

    if(len < 0){
        recv_ok.wait(recv_l);
        goto read_date;
    }
    return len;
}
