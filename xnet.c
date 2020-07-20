#include "xnet.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static int dial_net(int socktype, const char *ip, uint16_t port)
{
    int fd, rc;
    struct sockaddr_in addr;

    fd = socket(AF_INET, socktype, 0);
    if (fd == -1)
        return -1;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    rc = connect(fd, (struct sockaddr *)&addr, sizeof(addr));
    if (rc == -1) {
        close(fd);
        return -1;
    }
    return fd;
}

int dial_tcp(const char *ip, int port)
{
    return dial_net(SOCK_STREAM, ip, port);
}
