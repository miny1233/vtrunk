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
    std::vector<tunnel> socket_fd_list;
    std::vector<tunnel>::iterator s_tunnel,r_tunnel;
public:
    load_balance():socket_fd_list(),s_tunnel(socket_fd_list.begin()){};
    void add_tunnel(tunnel fd)
    {
        socket_fd_list.push_back(fd);
        s_tunnel = socket_fd_list.begin();
        r_tunnel = s_tunnel;
    }

    void del_tunnel(tunnel fd)
    {
        //socket_fd_list.erase(fd);
        //s_tunnel = socket_fd_list.begin();
        //r_tunnel = s_tunnel;
    }

    ssize_t send_next(const void* buf,size_t len)
    {
        if (s_tunnel == socket_fd_list.end())s_tunnel = socket_fd_list.begin();

        int ret = sendto(s_tunnel->fd,buf,len,0,&s_tunnel->objective,sizeof(s_tunnel->objective));

        s_tunnel++;
        if (ret == -1)
            std::cerr<<"send_next:"<<errno<<" len is "<<len<<std::endl;
        return ret;
    }

    ssize_t recv_next(void* buf,size_t buf_size)
    {
        if(r_tunnel == socket_fd_list.end())
            r_tunnel = socket_fd_list.begin();

        sockaddr none;
        socklen_t none_len;

        get_msg:
        ssize_t ret = recvfrom(r_tunnel->fd,buf,buf_size,MSG_DONTWAIT,&none,&none_len);

        if(ret == -1 && ++r_tunnel != socket_fd_list.end())
            goto get_msg;

        return ret;
    }

};


#endif //VTRUNK_LOAD_BALANCE_H
