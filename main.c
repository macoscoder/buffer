#include "io.h"
#include "xnet.h"
#include <ev.h>
#include <stdio.h>
#include <stdlib.h>

static void die(const char *s)
{
    perror(s);
    exit(1);
}

static void usage(const char *name)
{
    fprintf(stderr, "usage: %s ip port\n", name);
    exit(1);
}

static void completion()
{
    fprintf(stderr, "done\n");
}

static void error(const char *errmsg)
{
    fprintf(stderr, "error: %s\n", errmsg);
}

int main(int argc, char *argv[])
{
    struct ev_loop *loop;
    int connfd;
    struct io io;

    if (argc != 3)
        usage(argv[0]);

    connfd = dial_tcp(argv[1], atoi(argv[2]));
    if (connfd == -1)
        die("dial_tcp");

    loop = ev_default_loop(0);

    io_init(&io, loop, completion, error);
    io_copy(&io, connfd, 0);

    ev_run(loop, 0);
}
