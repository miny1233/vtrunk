//
// Created by 谢子南 on 2023/9/22.
//

#ifndef VTRUNK_TUNNEL_H
#define VTRUNK_TUNNEL_H

#include <sys/socket.h>
#include <set>

struct tunnel
{
    int fd;
    sockaddr objective;

    bool operator < (const tunnel& other) const
    {
        return this->fd < other.fd;
    }

};

#endif //VTRUNK_TUNNEL_H
