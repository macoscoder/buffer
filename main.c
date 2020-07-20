#include "buffer.h"
#include "xnet.h"
#include <ev.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_MAX_SIZE (1024 * 1024 * 8)

static struct buffer send_buf;

static ev_io stdin_watcher;
static ev_io socket_watcher;

static void die(const char *s)
{
    perror(s);
    exit(1);
}

static void usage(const char *name)
{
    fprintf(stderr, "%s ip\n", name);
    exit(1);
}

static void stdin_cb(struct ev_loop *loop, ev_io *w, int revents)
{
    char buf[4096];
    ssize_t nread;

    nread = read(w->fd, buf, sizeof(buf));
    switch (nread) {
    case -1:
        die("read stdin");
    case 0:
        ev_io_stop(loop, w);
        break;
    default:
        buffer_append_data(&send_buf, buf, nread);
        ev_io_start(loop, &socket_watcher);

        if (buffer_size(&send_buf) > BUFFER_MAX_SIZE)
            ev_io_stop(loop, w);
        break;
    }
}

static void socket_cb(struct ev_loop *loop, ev_io *w, int revents)
{
    ssize_t nwritten;

    printf("buffer len: %zu\n", buffer_length(&send_buf));
    printf("buffer size: %zu\n", buffer_size(&send_buf));
    printf("buffer room: %zu\n", buffer_room(&send_buf));

    nwritten = write(w->fd, buffer_data(&send_buf), buffer_length(&send_buf));
    if (nwritten == -1)
        die("write socket");

    printf("buffer drain: %zd\n", nwritten);

    buffer_drain(&send_buf, nwritten);
    if (buffer_length(&send_buf) == 0) {
        ev_io_stop(loop, &socket_watcher);
        buffer_reclaim(&send_buf);
        ev_io_start(loop, &stdin_watcher);
    }
}

int main(int argc, char *argv[])
{
    struct ev_loop *loop;
    int connfd;

    if (argc != 2)
        usage(argv[0]);

    connfd = dial_tcp(argv[1], 9999);
    if (connfd == -1)
        die("dial_tcp");

    buffer_init(&send_buf);

    loop = ev_default_loop(0);

    ev_io_init(&stdin_watcher, stdin_cb, 0, EV_READ);
    ev_io_start(loop, &stdin_watcher);

    ev_io_init(&socket_watcher, socket_cb, connfd, EV_WRITE);
    ev_io_start(loop, &socket_watcher);

    ev_run(loop, 0);
}
