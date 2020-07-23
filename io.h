#pragma once

#include "buffer.h"
#include <ev.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*completion_cb)();
typedef void (*error_cb)(const char *errmsg);

struct io {
    struct ev_io src_watcher;
    struct ev_io dst_watcher;
    struct buffer buf;
    struct ev_loop *loop;
    completion_cb completion;
    error_cb error;
    int src_done;
};

void io_init(struct io *io, struct ev_loop *loop, completion_cb completion, error_cb error);
void io_copy(struct io *io, int dst, int src);

#ifdef __cplusplus
}
#endif
