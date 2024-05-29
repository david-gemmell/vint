#ifndef VINT
#define VINT

#include <stdio.h>
#include <stdint.h>

extern int vint_read(FILE *stream, uint32_t *value);

extern int vint_write(FILE *stream, uint32_t value);

extern int vint_move(FILE *in_stream, FILE *out_stream, uint32_t count);

extern int vint_skip(FILE *stream, uint32_t count);

#endif