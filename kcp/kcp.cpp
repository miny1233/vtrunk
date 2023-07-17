
#include "kcp.h"

kcp::kcp(kcp_io &kcpIo,u_int32_t id,int mode):io(kcpIo){
    ikcp = ikcp_create(id,this);
    // 输出回调
    ikcp->output = [](const char *buf, int len, ikcpcb *kcp, void *user) -> int{
        auto* kcp_model = reinterpret_cast<decltype(this)>(user);
        return kcp_model->io.send(buf,len);
    };
    switch (mode) {
        case 0:ikcp_nodelay(ikcp,0, 10, 0, 0);
            break;
        case 1:ikcp_nodelay(ikcp, 0, 10, 0, 1);
            break;
        default:
            throw std::exception();
    }
    update_task = std::thread(&kcp::update,this);
    recv_task = std::thread(&kcp::recv_from_io,this);
    if(!update_task.joinable() || !recv_task.joinable())
        throw std::exception();
}

[[noreturn]] void kcp::update() {
    while (true) {
        ikcp_update(ikcp, kcp::iclock());
        //auto next_flush = ikcp_check(ikcp, iKcp::iclock());
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}

[[noreturn]] void kcp::recv_from_io() {
    char* buf = new char[BUFFER_SIZE];
    int len;
    while(true) {
        len = io.recv(buf,BUFFER_SIZE);
        if(len == -1)continue;  // -1是没有接收到 0为接收到长度是0的包
        ikcp_input(ikcp,buf,len);
        // 收到了数据 尝试能否读
        std::unique_lock<std::mutex> get_msg(recv_lock);
        recv_ok.notify_all();
    }
}

int kcp::send(const char* msg, int len) {
    return ikcp_send(ikcp,msg,len);
}

int kcp::recv(char* buf, int buf_size) {
    read_date:
    int len = ikcp_recv(ikcp,buf,buf_size);
    if(len == -1){
        std::unique_lock<std::mutex> recv_l (recv_lock);
        recv_ok.wait(recv_l);
        goto read_date;
    }
    return len;
}
