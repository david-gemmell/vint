#include <stdio.h>

#include "common.h"
#include "minunit.h"
#include "vint_tests.c"
#include "app_tests.c"

int tests_run = 0;

static char *all_tests() {
    mu_run_suite(vint_all_tests);
    mu_run_suite(app_all_tests);
    return NULL;
}

int main(int argc, char **argv) {
    char *result = all_tests();
    if (result) {
        LOGE("%s\n", result);
    } else {
        LOGS("ALL TESTS PASSED\n");
    }
    LOGI("Tests run: %d\n", tests_run);

    return result != 0;
}
