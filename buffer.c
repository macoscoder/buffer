#include "buffer.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline void *xrealloc(void *ptr, size_t size)
{
    void *p = realloc(ptr, size);
    if (!p) {
        fprintf(stderr, "realloc(%p, %zu): out of memory\n", ptr, size);
        exit(1);
    }
    return p;
}

static inline size_t buffer_headroom(const struct buffer *buf)
{
    return buf->data - buf->head;
}

static inline size_t buffer_tailroom(const struct buffer *buf)
{
    return buf->end - buf->tail;
}

static inline void move_data(struct buffer *buf)
{
    size_t data_len;

    data_len = buffer_length(buf);
    memmove(buf->head, buf->data, data_len);
    buf->data = buf->head;
    buf->tail = buf->data + data_len;
}

static void buffer_grow(struct buffer *buf, size_t len)
{
    size_t size, new_size, data_len;

    assert(buf->head == buf->data);

    size = buffer_size(buf) + len;
    new_size = 1024;
    while (new_size < size)
        new_size <<= 1;

    data_len = buffer_length(buf);
    buf->head = xrealloc(buf->head, new_size);
    buf->data = buf->head;
    buf->tail = buf->data + data_len;
    buf->end = buf->head + new_size;
}

static void grow_if_needed(struct buffer *buf, size_t len)
{
    goto inside;

    while (buffer_headroom(buf) > 0) {
        move_data(buf);
    inside:
        if (buffer_tailroom(buf) >= len)
            return;
    }

    buffer_grow(buf, len);
}

void buffer_init(struct buffer *buf)
{
    memset(buf, 0, sizeof(struct buffer));
}

void buffer_free(struct buffer *buf)
{
    free(buf->head);
    memset(buf, 0, sizeof(struct buffer));
}

size_t buffer_length(const struct buffer *buf)
{
    return buf->tail - buf->data;
}

size_t buffer_size(const struct buffer *buf)
{
    return buf->end - buf->head;
}

unsigned char *buffer_data(const struct buffer *buf)
{
    return buf->data;
}

void buffer_append_data(struct buffer *buf, const void *data, size_t len)
{
    grow_if_needed(buf, len);
    memcpy(buf->tail, data, len);
    buf->tail += len;
}

void buffer_append_null(struct buffer *buf)
{
    char null = '\0';
    buffer_append_data(buf, &null, 1);
}

void buffer_append_char(struct buffer *buf, char c)
{
    buffer_append_data(buf, &c, 1);
}

void buffer_append_printf(struct buffer *buf, const char *fmt, ...)
{
    va_list ap;
    int npr;
    char *str;

    va_start(ap, fmt);
    npr = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);

    str = alloca(npr);

    va_start(ap, fmt);
    vsprintf(str, fmt, ap);
    va_end(ap);

    buffer_append_data(buf, str, npr);
}

void buffer_drain(struct buffer *buf, size_t len)
{
    buf->data += len;
}

void buffer_reset(struct buffer *buf)
{
    buf->data = buf->tail = buf->head;
}

void buffer_hexdump(const struct buffer *buf)
{
    size_t len, i;

    len = buffer_length(buf);
    for (i = 0; i < len; i++)
        printf("%02x", buf->data[i]);
    printf("\n");
}
