#include "io.h"
#include <errno.h>
#include <unistd.h>

#define HIGH_WATER_MARK (1024 * 1024 * 4)
#define LOW_WATER_MARK  (1024 * 1024 * 1)

static void io_error(struct io *io)
{
    ev_io_stop(io->loop, &io->src_watcher);
    ev_io_stop(io->loop, &io->dst_watcher);

    if (io->error)
        io->error(strerror(errno));
}

static void io_completion(struct io *io)
{
    ev_io_stop(io->loop, &io->src_watcher);
    ev_io_stop(io->loop, &io->dst_watcher);

    if (io->completion)
        io->completion();
}

static void src_read_cb(struct ev_loop *loop, ev_io *w, int revents)
{
    char buf[4096];
    ssize_t nread;
    struct io *io;

    io = (struct io *)w->data;

    nread = read(w->fd, buf, sizeof(buf));
    switch (nread) {
    case -1:
        io_error(io);
        return;
    case 0:
        ev_io_stop(loop, &io->src_watcher);
        ev_io_start(loop, &io->dst_watcher);
        io->src_done = 1;
        return;
    default:
        buffer_append_data(&io->buf, buf, nread);
        ev_io_start(loop, &io->dst_watcher);

        if (buffer_length(&io->buf) > HIGH_WATER_MARK)
            ev_io_stop(loop, &io->src_watcher);
        return;
    }
}

static void dst_write_cb(struct ev_loop *loop, ev_io *w, int revents)
{
    ssize_t nwritten;
    struct io *io;

    io = (struct io *)w->data;

    nwritten = write(w->fd, buffer_data(&io->buf), buffer_length(&io->buf));
    if (nwritten == -1) {
        io_error(io);
        return;
    }

    buffer_drain(&io->buf, nwritten);

    if ((buffer_length(&io->buf) < LOW_WATER_MARK) && !io->src_done)
        ev_io_start(loop, &io->src_watcher);

    if ((buffer_length(&io->buf) == 0) && io->src_done)
        io_completion(io);
}

void io_init(struct io *io, struct ev_loop *loop, completion_cb completion, error_cb error)
{
    buffer_init(&io->buf);

    io->loop = loop;
    io->completion = completion;
    io->error = error;
    io->src_done = 0;
}

void io_copy(struct io *io, int dst, int src)
{
    ev_io_init(&io->src_watcher, src_read_cb, src, EV_READ);
    ev_io_init(&io->dst_watcher, dst_write_cb, dst, EV_WRITE);

    io->src_watcher.data = io;
    io->dst_watcher.data = io;

    ev_io_start(io->loop, &io->src_watcher);
    ev_io_start(io->loop, &io->dst_watcher);
}
