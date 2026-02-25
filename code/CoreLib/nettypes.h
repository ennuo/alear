#pragma once

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>

#include <netex/errno.h>
#define net_errno sys_net_errno
#define EWOULDBLOCK SYS_NET_EWOULDBLOCK

typedef int SOCKET;

inline int select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, timeval* timeout)
{
    return socketselect(nfds, readfds, writefds, exceptfds, timeout);
}

inline void close(int s) { socketclose(s); }
