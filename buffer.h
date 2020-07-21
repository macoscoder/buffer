#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct buffer {
    unsigned char *head, *data, *tail, *end;
};

void buffer_init(struct buffer *buf);
void buffer_free(struct buffer *buf);

size_t buffer_length(const struct buffer *buf);
size_t buffer_size(const struct buffer *buf);
unsigned char *buffer_data(const struct buffer *buf);

void buffer_append_data(struct buffer *buf, const void *data, size_t len);
void buffer_append_zero(struct buffer *buf, size_t len);
void buffer_append_null(struct buffer *buf);
void buffer_append_char(struct buffer *buf, char c);
void buffer_append_printf(struct buffer *buf, const char *fmt, ...);

void buffer_drain(struct buffer *buf, size_t len);
void buffer_reset(struct buffer *buf);

void buffer_hexdump(const struct buffer *buf);

#ifdef __cplusplus
}
#endif
