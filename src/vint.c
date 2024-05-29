#include "vint.h"

extern int vint_read(FILE *stream, uint32_t *value) {
    register int data = fgetc(stream);
    if (data == EOF) {
        return EOF;
    }
    register uint8_t b = (uint8_t)data;
    if (b == (uint8_t)0x80) {
        fprintf(stderr, "Attempting to read null value as int\n");
        return EOF;
    }
    *value = b & 0x7F;
    while ((b & 0x80) != 0) {
        data = getc(stream);
        if (data == -1) {
            fprintf(stderr, "Unexpected end of stream\n");
            return EOF;
        }
        b = (uint8_t)data;
        *value <<= 7;
        *value |= (b & 0x7F);
    }
    return 0;
}

extern int vint_write(FILE *stream, uint32_t value) {
    int code = 0;
    if (value > 0x0FFFFFFF || value < 0) {
        code |= putc((uint8_t)(0x80 | (value >> 28)), stream);
    }
    if (value > 0x1FFFFF || value < 0) {
        code |= putc((uint8_t)(0x80 | ((value >> 21) & 0x7F)), stream);
    }
    if (value > 0x3FFF || value < 0) {
        code |= putc((uint8_t)(0x80 | ((value >> 14) & 0x7F)), stream);
    }
    if (value > 0x7F || value < 0) {
        code |= putc((uint8_t)(0x80 | ((value >> 7) & 0x7F)), stream);
    }
    code |= putc((uint8_t)(value & 0x7F), stream);
    return code;
}

extern int vint_move(FILE *in_stream, FILE *out_stream, uint32_t count) {
    int code = 0;
    for (uint32_t i = 0; i < count; ++i) {
        register int data = getc(in_stream);
        if (data == -1) {
            fprintf(stderr, "Unexpected end of stream\n");
            return EOF;
        }
        register uint8_t b = (uint8_t)data;
        code |= putc(b, out_stream);
        if (b == (uint8_t)0x80) {
            fprintf(stderr, "Attempting to read null value as int\n");
            return EOF;
        }
        while ((b & 0x80) != 0) {
            data = getc(in_stream);
            if (data == -1) {
                fprintf(stderr, "Unexpected end of stream\n");
                return EOF;
            }
            b = (uint8_t)data;
            code |= putc(b, out_stream);
        }
    }
    return 0;
}

extern int vint_skip(FILE *stream, uint32_t count) {
    uint32_t v;
    for (uint32_t i = 0; i < count; ++i) {
        if (vint_read(stream, &v) == EOF) {
            return EOF;
        }
    }
}