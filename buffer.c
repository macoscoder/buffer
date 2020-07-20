#include "buffer.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static void grow_if_needed(struct buffer *buf, size_t len)
{
    size_t size, new_size, data_len;

    if (buffer_tailroom(buf) >= len)
        return;
    if (buffer_headroom(buf) > 0) {
        move_data(buf);
        if (buffer_tailroom(buf) >= len)
            return;
    }

    size = buffer_size(buf) + len;
    new_size = 1024;
    while (new_size < size)
        new_size <<= 1;

    data_len = buffer_length(buf);
    buf->head = realloc(buf->head, new_size);
    buf->data = buf->head;
    buf->tail = buf->data + data_len;
    buf->end = buf->head + new_size;
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

size_t buffer_room(const struct buffer *buf)
{
    return buffer_headroom(buf) + buffer_tailroom(buf);
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

void buffer_append_zero(struct buffer *buf, size_t len)
{
    char data[len];
    memset(data, 0, len);
    buffer_append_data(buf, data, len);
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

void buffer_reclaim(struct buffer *buf)
{
    size_t data_len;

    move_data(buf);
    data_len = buffer_length(buf);
    buf->head = buf->data = realloc(buf->head, data_len);
    buf->end = buf->tail = buf->head + data_len;
}

void buffer_hexdump(const struct buffer *buf)
{
    size_t len, i;

    len = buffer_length(buf);
    for (i = 0; i < len; i++)
        printf("%02x", buf->data[i]);
    printf("\n");
}
