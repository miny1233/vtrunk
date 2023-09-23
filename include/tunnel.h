//
// Created by 谢子南 on 2023/9/22.
//

#ifndef VTRUNK_TUNNEL_H
#define VTRUNK_TUNNEL_H

#include <sys/socket.h>
#include <set>
#include <initializer_list>

class tunnel
{
private:
    int number;
public:
    static int num_increase;
    int fd;
    sockaddr objective;

    tunnel(int fd,sockaddr objective):fd(fd), objective(objective){
        number = tunnel::num_increase++;
        std::cout<<std::format("[{}] Create Tunnel! num = {} \n",__FILE_NAME__,number);
    }

    bool operator < (const tunnel& other) const
    {
        return this->number < other.number;
    }

};

int tunnel::num_increase = 1;

#endif //VTRUNK_TUNNEL_H
