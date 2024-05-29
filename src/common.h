#ifndef COMMON
#define COMMON

#define ANSI_RED "\x1B[31m"
#define ANSI_YELLOW "\x1B[33m"
#define ANSI_GREEN "\x1B[32m"
#define ANSI_WHITE "\x1B[37m"
#define ANSI_RESET "\x1B[0m"
#define LOGE(...)                   \
    fprintf(stderr, ANSI_RED);         \
    fprintf(stderr, __VA_ARGS__); \
    fprintf(stderr, ANSI_RESET)
#define LOGW(...)                   \
    fprintf(stderr, ANSI_YELLOW);      \
    fprintf(stderr, __VA_ARGS__); \
    fprintf(stderr, ANSI_RESET)
#define LOGS(...)                   \
    fprintf(stderr, ANSI_GREEN);       \
    fprintf(stderr, __VA_ARGS__); \
    fprintf(stderr, ANSI_RESET)
#define LOGI(...)                   \
    fprintf(stderr, ANSI_WHITE);       \
    fprintf(stderr, __VA_ARGS__); \
    fprintf(stderr, ANSI_RESET)
#define PERROR(S)              \
    fprintf(stderr, ANSI_RED); \
    perror((S));               \
    fprintf(stderr, ANSI_RESET)
#define FAIL_IF_NULL(A, S...) do {if ((A) == NULL) { LOGE(S); exit(1);}} while(0)
    
#define NOP __asm("nop;\n")

#endif