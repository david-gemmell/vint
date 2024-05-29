#include <stdlib.h>
#include <string.h>

#include "minunit.h"
#include "vint.h"

static char *test_read_write() {
    char *buf;
    size_t buflen;
    int size = 7;
    uint32_t values[] = {3, 130, 16386, 2097154, 268435458, 2147483647, -5};
    FILE *fd = open_memstream(&buf, &buflen);
    for (int i = 0; i < size; ++i) {
        vint_write(fd, values[i]);
    }
    fclose(fd);

    uint32_t value = 0;
    fd = fmemopen(buf, buflen, "r");
    // fd = fopen("test.out", "r");
    for (int i = 0; i < size; ++i) {
        vint_read(fd, &value);
        printf("%d\n", value);
        mu_assert("error, read value != write value", value == values[i]);
    }
    fclose(fd);

    free(buf);
    return NULL;
}

static char *test_move() {
    char *buf;
    size_t buflen;
    int size = 7;
    uint32_t values[] = {3, 130, 16386, 2097154, 268435458, 2147483647, -5};
    FILE *fd = open_memstream(&buf, &buflen);
    for (int i = 0; i < 7; ++i) {
        vint_write(fd, values[i]);
    }
    fclose(fd);

    fd = fmemopen(buf, buflen, "r");
    char *buf2;
    size_t buflen2;
    FILE *fd2 = open_memstream(&buf2, &buflen2);
    vint_move(fd, fd2, size);
    fclose(fd);
    fclose(fd2);

    uint32_t value;
    fd = fmemopen(buf2, buflen2, "r");
    for (int i = 0; i < 7; ++i) {
        vint_read(fd, &value);
        printf("%d\n", value);
        mu_assert("error, read value != write value", value == values[i]);
    }
    fclose(fd);

    free(buf);
    free(buf2);
    return NULL;
}

static char *test_skip() {
    char *buf;
    size_t buflen;
    int size = 7;
    uint32_t values[] = {3, 130, 16386, 2097154, 268435458, 2147483647, -5};
    FILE *fd = open_memstream(&buf, &buflen);
    for (int i = 0; i < 7; ++i) {
        vint_write(fd, values[i]);
    }
    fclose(fd);

    uint32_t value;
    fd = fmemopen(buf, buflen, "r");
    for (int i = 0; i < 7; ++i) {
        if (i % 2 == 0) {
            vint_skip(fd, 1);
        } else {
            vint_read(fd, &value);
            printf("%d\n", value);
            mu_assert("error, read value != write value", value == values[i]);
        }
    }
    fclose(fd);

    free(buf);
    return NULL;
}

static char *vint_all_tests() {
    mu_run_test(test_read_write);
    mu_run_test(test_move);
    mu_run_test(test_skip);
    return NULL;
}