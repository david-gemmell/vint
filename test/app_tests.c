#include <stdlib.h>
#include <string.h>

#include "app.c"
#include "minunit.h"

static char *test_parse_args() {
    int argc = 2;
    char *argv[3] = {"app_name", "-e"};
    Options options = DEFAULT_OPTIONS;
    mu_assert("Encode should be false by default", !options.binary);
    parse_args(argc, argv, &options);
    mu_assert("Option '-e' doesn't applied", options.encode);

    argv[1] = "-d";
    parse_args(argc, argv, &options);
    mu_assert("Option '-d' doesn't applied", !options.encode);
    mu_assert("Binary should be false by default", !options.binary);

    argv[1] = "-b";
    parse_args(argc, argv, &options);
    mu_assert("Option '--binary' doesn't applied", options.binary);

    argv[1] = "-t";
    parse_args(argc, argv, &options);
    mu_assert("Option '--text' doesn't applied", !options.binary);

    mu_assert("Header should be false by default", options.header);
    argv[1] = "--no-header";
    parse_args(argc, argv, &options);
    mu_assert("Option '--no-header' doesn't applied", !options.header);

    mu_assert("Format should be false by default", !options.format);
    mu_assert("Scheme should be NULL by default", options.scheme == NULL);

    argv[1] = "--scheme=scheme/path";
    parse_args(argc, argv, &options);
    mu_assert("Option '--scheme' doesn't applied", options.format);
    mu_assert("Option '--scheme' doesn't applied", strcmp(options.scheme, "scheme/path") == 0);
    free(options.scheme);
    options.format = 0;
    options.scheme = NULL;

    mu_assert("File should be false by default", !options.file);
    mu_assert("File name should be NULL by default", options.file_name == NULL);

    argv[1] = "--file=file/path";
    parse_args(argc, argv, &options);
    mu_assert("Option '--file' doesn't applied", options.file);
    mu_assert("Option '--file' doesn't applied", strcmp(options.file_name, "file/path") == 0);
    free(options.file_name);
    options.file = 0;
    options.file_name = NULL;

    argv[1] = "-";
    parse_args(argc, argv, &options);
    mu_assert("Not supported - for stdin", !options.file);

    argv[1] = "file/path";
    parse_args(argc, argv, &options);
    mu_assert("Option '--file' doesn't applied", options.file);
    mu_assert("Option '--file' doesn't applied", strcmp(options.file_name, "file/path") == 0);

    mu_assert("Column separator should be '\\t' by default", options.column_sep == '\t');
    mu_assert("List separator should be ':' by default", options.list_sep == ':');

    argv[1] = "--column_sep= ";
    parse_args(argc, argv, &options);
    mu_assert("Option '--column_sep' doesn't applied", options.column_sep == ' ');

    argv[1] = "--list_sep=,";
    parse_args(argc, argv, &options);
    mu_assert("Option '--list_sep' doesn't applied", options.list_sep == ',');

    argc = 3;
    argv[1] = "-s";
    argv[2] = "scheme/path";
    parse_args(argc, argv, &options);
    mu_assert("Option '--scheme' doesn't applied", options.format);
    mu_assert("Option '--scheme' doesn't applied", strcmp(options.scheme, "scheme/path") == 0);

    argc = 2;
    argv[1] = "--version";
    parse_args(argc, argv, &options);
    mu_assert("Option '--version' doesn't applied", options.version);

    argc = 2;
    argv[1] = "-h";
    parse_args(argc, argv, &options);
    mu_assert("Option '-h' doesn't applied", options.help);

    return NULL;
}

static void clearScheme(Scheme *scheme) {
    for (int i = 0; i < scheme->size; ++i) {
        if (scheme->columns[i] != NULL) {
            free(scheme->columns[i]->name);
            free(scheme->columns[i]);
            scheme->columns[i] = NULL;
        }
    }
    free(scheme->columns);
    scheme->size = 0;
    scheme->_size = 0;
}

static char *test_parse_scheme() {
    Scheme scheme = DEFAULT_SCHEME;
    parse_scheme("col1", &scheme);
    mu_assert("Can parse 'col1'", scheme.size == 1);
    mu_assert("Name doesn't equal 'col1'", strcmp(scheme.columns[0]->name, "col1") == 0);
    mu_assert("Column hidden flag should be false", !scheme.columns[0]->hidden);
    mu_assert("Column array flag should be false", !scheme.columns[0]->array);
    mu_assert("Column diff flag should be false", !scheme.columns[0]->diff);

    parse_scheme("col1,col2:a", &scheme);
    mu_assert("Can parse 'col1,col2:a'", scheme.size == 2);
    mu_assert("Name doesn't equal 'col2'", strcmp(scheme.columns[1]->name, "col2") == 0);
    mu_assert("Column hidden flag should be false", !scheme.columns[1]->hidden);
    mu_assert("Column array flag should be true", scheme.columns[1]->array);
    mu_assert("Column diff flag should be false", !scheme.columns[1]->diff);

    parse_scheme("col1,col2:d", &scheme);
    mu_assert("Can parse 'col1,col2:d'", scheme.size == 2);
    mu_assert("Name doesn't equal 'col2'", strcmp(scheme.columns[1]->name, "col2") == 0);
    mu_assert("Column hidden flag should be false", !scheme.columns[1]->hidden);
    mu_assert("Column array flag should be false", !scheme.columns[1]->array);
    mu_assert("Column diff flag should be true", scheme.columns[1]->diff);

    parse_scheme("col1,col2:a,col3:h", &scheme);
    mu_assert("Can parse 'col1,col2:a,col3:h'", scheme.size == 3);
    mu_assert("Name doesn't equal 'col3'", strcmp(scheme.columns[2]->name, "col3") == 0);
    mu_assert("Column hidden flag should be true", scheme.columns[2]->hidden);
    mu_assert("Column array flag should be false", !scheme.columns[2]->array);
    mu_assert("Column diff flag should be false", !scheme.columns[2]->diff);

    parse_scheme("col1,col2:ah,col3", &scheme);
    mu_assert("Can parse 'col1,col2:ah,col3'", scheme.size == 3);
    mu_assert("Column hidden flag should be true", scheme.columns[1]->hidden);
    mu_assert("Column array flag should be true", scheme.columns[1]->array);
    mu_assert("Column diff flag should be false", !scheme.columns[1]->diff);

    parse_scheme("col1,col2:ha,col3", &scheme);
    mu_assert("Can parse 'col1,col2:ah,col3'", scheme.size == 3);
    mu_assert("Column hidden flag should be true", scheme.columns[1]->hidden);
    mu_assert("Column array flag should be true", scheme.columns[1]->array);
    mu_assert("Column diff flag should be false", !scheme.columns[1]->diff);

    parse_scheme("col1,col2,col3,col4,col5,col6,col7,col8,col9,col10,col11", &scheme);
    mu_assert("Can parse 'col1,col2,col3,col4,col5,col6,col7,col8,col9,col10,col11'", scheme.size == 11);
    mu_assert("Name doesn't equal 'col1'", strcmp(scheme.columns[0]->name, "col1") == 0);
    mu_assert("Name doesn't equal 'col11'", strcmp(scheme.columns[10]->name, "col11") == 0);
    mu_assert("Column hidden flag should be false", !scheme.columns[10]->hidden);
    mu_assert("Column array flag should be false", !scheme.columns[10]->array);
    mu_assert("Column diff flag should be false", !scheme.columns[10]->diff);

    clearScheme(&scheme);

    return NULL;
}

static char *test_read_next_int() {
    uint32_t numbers[] = {1, 100, 2, 200, 300, 400};
    int size = 6; 
    char input[] = "1 100 2 200,300 400\0";
    FILE *fd = fmemopen(input, strlen(input), "r");

    int i = 0;
    uint32_t value;
    while (size-- > 0 && read_next_int(fd, &value) != EOF) {
        mu_assert("read_next_int return bad value", value == numbers[i++]);
    }
    fclose(fd);

    return 0;
}

static char *test_encode_decode_binary() {
    char *mbuf, *ebuf;
    size_t mbuflen, ebuflen;
    char input[] = "Hello world!\0";
    FILE *sfd = fmemopen(input, strlen(input), "r");
    FILE *mfd = open_memstream(&mbuf, &mbuflen);
    encode_binary(sfd, mfd);
    fclose(sfd);
    fclose(mfd);

    mfd = fmemopen(mbuf, mbuflen, "r");
    FILE *efd = open_memstream(&ebuf, &ebuflen);
    decode_to_binary(mfd, efd);
    fclose(mfd);
    fclose(efd);

    mu_assert("Decoded binary is not equal to the input binary", strcmp(input, ebuf) == 0);

    free(mbuf);
    free(ebuf);
}

static char *test_encode_decode_text() {
    char *mbuf, *ebuf;
    size_t mbuflen, ebuflen;
    char input[] = "123 321 \0";
    FILE *sfd = fmemopen(input, strlen(input), "r");
    FILE *mfd = open_memstream(&mbuf, &mbuflen);
    encode_text(sfd, mfd);
    fclose(sfd);
    fclose(mfd);

    mfd = fmemopen(mbuf, mbuflen, "r");
    FILE *efd = open_memstream(&ebuf, &ebuflen);
    decode_to_text(' ', mfd, efd);
    fclose(mfd);
    fclose(efd);

    mu_assert("Decoded text is not equal to the input text", strcmp(input, ebuf) == 0);

    free(mbuf);
    free(ebuf);
}

static char *test_encode_decode_formatted_text() {
    Options options = DEFAULT_OPTIONS;
    options.format = 1;
    options.header = 1;
    options.scheme = "col1:a,col2:d,col3,col4:h";
    options.column_sep = ' ';
    options.list_sep = ':';
    Scheme scheme = DEFAULT_SCHEME;
    
    parse_scheme(options.scheme, &scheme);

    char *mbuf, *ebuf;
    size_t mbuflen, ebuflen;
    char input[] = "col1 col2 col3\n1 100 2 200,300 400\0";
    FILE *sfd = fmemopen(input, strlen(input), "r");
    FILE *mfd = open_memstream(&mbuf, &mbuflen);
    encode_formatted_text(&options, &scheme, sfd, mfd);
    fclose(sfd);
    fclose(mfd);

    mfd = fmemopen(mbuf, mbuflen, "r");
    FILE *efd = open_memstream(&ebuf, &ebuflen);
    decode_to_formatted_text(&options, &scheme, mfd, efd);
    //decode_to_text(' ', mfd, efd);
    fclose(mfd);
    fclose(efd);

    char output[] = "col1 col2 col3\n100 200:300 400\0";
    mu_assert("Decoded formatted text is not equal to the input formatted text", strcmp(output, ebuf) == 0);

    clearScheme(&scheme);
    free(mbuf);
    free(ebuf);
}

static char *app_all_tests() {
    mu_run_test(test_parse_args);
    mu_run_test(test_parse_scheme);
    mu_run_test(test_read_next_int);
    mu_run_test(test_encode_decode_binary);
    mu_run_test(test_encode_decode_text);
    mu_run_test(test_encode_decode_formatted_text);
    return NULL;
}