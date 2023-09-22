//
// Created by 谢子南 on 2023/9/22.
//

#ifndef VTRUNK_LOAD_BALANCE_H
#define VTRUNK_LOAD_BALANCE_H

#include <sys/socket.h>
#include <set>
#include <tunnel.h>

class load_balance {
private:
    std::set<tunnel> socket_fd_list;
    std::set<tunnel>::iterator s_tunnel;
public:
    load_balance():socket_fd_list(),s_tunnel(socket_fd_list.begin()){};
    void add_tunnel(tunnel fd)
    {
        s_tunnel = socket_fd_list.insert(fd).first;
    }
    void del_tunnel(tunnel fd)
    {
        socket_fd_list.erase(fd);
    }
    ssize_t send_next(const void* buf,size_t len)
    {
        if (s_tunnel == socket_fd_list.end())s_tunnel = socket_fd_list.begin();
        return sendto(s_tunnel->fd,buf,len,MSG_DONTWAIT,&s_tunnel->objective,sizeof(s_tunnel->objective));
    }

    ssize_t recv_next(void* buf,size_t buf_size)
    {

    }

};


#endif //VTRUNK_LOAD_BALANCE_H
