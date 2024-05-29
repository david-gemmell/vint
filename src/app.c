#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "vint.h"

#define PROGRAM_NAME "vint"
#define PROGRAM_VERSION "1.0.0"
#define DEFAULT_OPTIONS \
    { 0, 0, 0, 0, 0, 1, 0, NULL, NULL, '\t', ':' }
#define DEFAULT_SCHEME \
    { NULL, 0, 0 }

typedef struct {
    uint8_t help : 1;
    uint8_t version : 1;
    uint8_t encode : 1;
    uint8_t binary : 1;
    uint8_t format : 1;
    uint8_t header : 1;
    uint8_t file : 1;
    char *file_name;
    char *scheme;
    char column_sep;
    char list_sep;
} Options;

typedef struct {
    uint8_t array : 1;
    uint8_t diff : 1;
    uint8_t hidden : 1;
    char *name;
} Column;

typedef struct {
    Column **columns;
    int size;
    int _size;
} Scheme;

static void print_help() {
    fprintf(stdout,"\
Usage: %s [OPTION]... [FILE]...\n\
  or:  %s [OPTION]... --file=F\n\
Variable-length quantity(VLQ) 7 bits per byte encoder/decoder for 32-bit integers\n\
\n\
With no FILE, or when FILE is -, read standard input.\n\
\n\
  -e, --encode           encode input stream to standard output\n\
  -d, --decode           decode input stream to standard output(used by default)\n\
  -b, --binary           interprets input stream for encoder and output stream for\n\
                         decoder as a list of 32-bit integers\n\
  -t, --text             interprets input stream for encoder and an output stream for\n\
                         decoder as a list of text integers\n\
      --file             read input from the file\n\
	                     If F is - then read names from standard input\n\
  -s, --scheme           specifies the interpretation scheme for the text format\n\
                         scheme format <column_name>[[:[a][d][h]],<column_name>...]\n\
                         a - the column is a list of numbers where the first number in\n\
                             the stream means the size of the list\n\
                         d - the column is a list of numbers where the first number in \n\
                             the stream means the size of the list. The numbers themselves\n\
                             are stored as the difference from the previous one\n\
                         h - hide column during processing\n\
      --no-header        skip the first line for encoder and don't print the header for decoder\n\
	  --column_sep       column separator for decoder\n\
	  --list_sep         list separator for decoder\n\
  -h, --help     display this help and exit\n\
      --version  output version information and exit\n\n",
            PROGRAM_NAME, PROGRAM_NAME);
}

static void print_version() { fprintf(stdout, "%s %s\n", PROGRAM_NAME, PROGRAM_VERSION); }

static void parse_args(int argc, char *argv[], Options *options) {
    int last_index = argc - 1;
    for (int i = 1; i < argc; ++i) {
        char *arg = argv[i];
        if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
            options->help = 1;
        } else if (strcmp(arg, "--version") == 0) {
            options->version = 1;
        } else if (strcmp(arg, "-e") == 0 || strcmp(arg, "--encode") == 0) {
            options->encode = 1;
        } else if (strcmp(arg, "-d") == 0 || strcmp(arg, "--decode") == 0) {
            options->encode = 0;
        } else if (strcmp(arg, "-b") == 0 || strcmp(arg, "--binary") == 0) {
            options->binary = 1;
        } else if (strcmp(arg, "-t") == 0 || strcmp(arg, "--text") == 0) {
            options->binary = 0;
        } else if (strcmp(arg, "-s") == 0) {
            options->format = 1;
            if (i < last_index) {
                options->scheme = argv[++i];
            }
        } else if (strncmp(arg, "--scheme=", 9) == 0) {
            options->format = 1;
            char *scheme;
            FAIL_IF_NULL(scheme = malloc(sizeof(char) * (strlen(arg) - 8)), "'scheme' malloc failed");
            options->scheme = strcpy(scheme, &arg[9]);
        } else if (strcmp(arg, "--no-header") == 0) {
            options->header = 0;
        } else if (strncmp(arg, "--file=", 7) == 0) {
            options->file = 1;
            char *file_name;
            FAIL_IF_NULL(file_name = malloc(sizeof(char) * (strlen(arg) - 6)), "'file_name' malloc failed");
            options->file_name = strcpy(file_name, &arg[7]);
        } else if (strncmp(arg, "--column_sep=", 13) == 0) {
            if (strlen(arg) == 14 && (arg[13] < '0' || arg[13] > '9')) {
                options->column_sep = arg[13];
            } else {
                LOGW("Bad argument '%s'\n", arg);
            }
        } else if (strncmp(arg, "--list_sep=", 11) == 0) {
            if (strlen(arg) == 12 && (arg[11] < '0' || arg[11] > '9')) {
                options->list_sep = arg[11];
            } else {
                LOGW("Bad argument '%s'\n", arg);
            }
        } else if (i == last_index && options->file_name == NULL) {
            if (strcmp(arg, "-")) {
                options->file = 1;
                options->file_name = arg;
            }
        } else {
            LOGW("Unknown argument '%s'\n", arg);
        }
    }
}

static void parse_scheme(char *scheme_str, Scheme *scheme) {
    if (scheme->_size == 0) {
        FAIL_IF_NULL(scheme->columns = malloc(10 * sizeof(Column *)), "'columns' malloc failed");
        scheme->_size = 10;
        scheme->size = 0;
        for (int i = 0; i < scheme->_size; ++i) {
            scheme->columns[i] = NULL;
        }
    } else {
        for (int i = 0; i < scheme->size; ++i) {
            if (scheme->columns[i] != NULL) {
                free(scheme->columns[i]->name);
                free(scheme->columns[i]);
            }
            scheme->columns[i] = NULL;
        }
        scheme->size = 0;
    }

    char ch;
    int i = 0;
    while ((ch = scheme_str[i]) != '\0') {
        int j = i;
        while ((ch = scheme_str[j]) != ':' && ch != ',' && ch != '\0') {
            ++j;
        }

        int l = j - i;
        Column *column;
        FAIL_IF_NULL(column = malloc(sizeof(Column)), "'column' malloc failed");
        FAIL_IF_NULL(column->name = malloc((l + 1) * sizeof(char)), "'name' malloc failed");
        column->name[l] = '\0';
        column->array = 0;
        column->diff = 0;
        column->hidden = 0;
        strncpy(column->name, &scheme_str[i], l);

        if (ch == ':') {
            ++j;
            while ((ch = scheme_str[j]) != ',' && ch != '\0') {
                if (ch == 'd') {
                    column->diff = 1;
                    column->array = 0;
                } else if (ch == 'a') {
                    column->array = 1;
                    column->diff = 0;
                } else if (ch == 'h') {
                    column->hidden = 1;
                } else {
                    LOGW("Unknown qualifier %c\n", ch);
                }
                ++j;
            }
        }

        if (scheme->size == scheme->_size) {
            FAIL_IF_NULL(scheme->columns = realloc(scheme->columns, (scheme->_size + 10) * sizeof(Column *)),
                         "Can't realloc memory");

            for (int i = scheme->_size; i < (scheme->_size + 10); ++i) {
                scheme->columns[i] = NULL;
            }
            scheme->_size += 10;
        }

        scheme->columns[scheme->size++] = column;

        i = j;
        if (ch != '\0') {
            ++i;
        }
    }
}

static int read_next_int(FILE *stream, uint32_t *value) {
    if (feof(stream)) {
        return EOF;
    }

    int ch;
    while ((ch = fgetc(stream)) != EOF) {
        if (ch >= '0' && ch <= '9') {
            *value = ch - '0';
            while ((ch = fgetc(stream)) >= '0' && ch <= '9') {
                *value *= 10;
                *value += ch - '0';
            }
            return 0;
        }
    }
    return EOF;
}

static void encode_binary(FILE *in_stream, FILE *out_stream) {
    uint32_t value;
    int ch, i;
    while ((ch = fgetc(in_stream)) != EOF) {
        i = 0;
        value = ch & 0xFF;
        while (i < 3 && (ch = fgetc(in_stream)) != EOF) {
            value <<= 8;
            value |= ch & 0xFF;
            i++;
        }
        if (i != 3) {
            LOGW("The length of the input file is not a multiple of 4 bytes\n");
        }
        vint_write(out_stream, value);
    }
}

static void encode_text(FILE *in_stream, FILE *out_stream) {
    uint32_t value = 0;
    while (read_next_int(in_stream, &value) != EOF) {
        vint_write(out_stream, value);
    }
}

static void encode_formatted_text(Options *options, Scheme *scheme, FILE *in_stream, FILE *out_stream) {
    if (options->header) {
        char ch;
        while ((ch = fgetc(in_stream)) != EOF && ch != '\n') {
        }
    }

    int i = 0;
    uint32_t value, size, diff;
    while (read_next_int(in_stream, &value) != EOF) {
        Column column = *(scheme->columns[i]);
        if (column.array) {
            size = value;
            if (!column.hidden) {
                vint_write(out_stream, value);
            }
            while (size-- > 0 && read_next_int(in_stream, &value) != EOF) {
                if (!column.hidden) {
                    vint_write(out_stream, value);
                }
            }
        } else if (column.diff) {
            diff = 0;
            size = value;
            if (!column.hidden) {
                vint_write(out_stream, value);
            }
            while (size-- > 0 && read_next_int(in_stream, &value) != EOF) {
                if (!column.hidden) {
                    diff = value - diff;
                    vint_write(out_stream, diff);
                }
            }
        } else if (!column.hidden) {
            vint_write(out_stream, value);
        }

        if (++i == scheme->size) {
            i = 0;
        }
    }
}

static void decode_to_binary(FILE *in_stream, FILE *out_stream) {
    uint32_t value;
    while (vint_read(in_stream, &value) != EOF) {
        fputc((value >> 24) & 0xff, out_stream);
        fputc((value >> 16) & 0xff, out_stream);
        fputc((value >> 8) & 0xff, out_stream);
        fputc(value & 0xff, out_stream);
    }
}

static void decode_to_text(char sep, FILE *in_stream, FILE *out_stream) {
    uint32_t value;
    while (vint_read(in_stream, &value) != EOF) {
        fprintf(out_stream, "%u", value);
        fputc(sep, out_stream);
    }
    fputc('\n', out_stream);
}

static void decode_to_formatted_text(Options *options, Scheme *scheme, FILE *in_stream, FILE *out_stream) {
    if (options->header) {
        for (int i = 0; i < scheme->size; ++i) {
            if (!scheme->columns[i]->hidden) {
                if (i != 0) {
                    fputc(options->column_sep, out_stream);
                }
                fputs(scheme->columns[i]->name, out_stream);
            }
        }
        fputc('\n', out_stream);
    }

    int i = 0;
    uint32_t value, size, diff;
    while (vint_read(in_stream, &value) != EOF) {
        if (i != 0) {
            fputc(options->column_sep, out_stream);
        }

        Column column = *(scheme->columns[i]);
        if (column.array) {
            size = value;
            while (size-- > 0 && vint_read(in_stream, &value) != EOF) {
                if (!column.hidden) {
                    fprintf(out_stream, "%u", value);
                    if (size) {
                        fputc(options->list_sep, out_stream);
                    }
                }
            }
        } else if (column.diff) {
            diff = 0;
            size = value;
            while (size-- > 0 && vint_read(in_stream, &value) != EOF) {
                if (!column.hidden) {
                    diff += value;
                    fprintf(out_stream, "%u", diff);
                    if (size) {
                        fputc(options->list_sep, out_stream);
                    }
                }
            }

        } else if (!column.hidden) {
            fprintf(out_stream, "%u", value);
        }

        if (++i == scheme->size) {
            i = 0;
            fputc('\n', out_stream);
        }
    }
}

static int launch(int argc, char *argv[]) {
    Options options = DEFAULT_OPTIONS;
    parse_args(argc, argv, &options);

    if (options.help) {
        print_help();
        return 0;
    }

    if (options.version) {
        print_version();
        return 0;
    }

    errno = 0;
    FILE *file = options.file ? fopen(options.file_name, "r") : stdin;

    if (file) {
        if (options.encode) {
            if (options.binary) {
                encode_binary(file, stdout);
            } else {
                if (options.format) {
                    Scheme scheme = DEFAULT_SCHEME;
                    parse_scheme(options.scheme, &scheme);
                    encode_formatted_text(&options, &scheme, file, stdout);
                } else {
                    encode_text(file, stdout);
                }
            }
        } else {
            if (options.binary) {
                decode_to_binary(file, stdout);
            } else {
                if (options.format) {
                    Scheme scheme = DEFAULT_SCHEME;
                    parse_scheme(options.scheme, &scheme);
                    decode_to_formatted_text(&options, &scheme, file, stdout);
                } else {
                    decode_to_text(options.column_sep, file, stdout);
                }
            }
        }
    } else {
        PERROR("FILE");
        return errno;
    }

    if (options.file) {
        fclose(file);
    }

    return 0;
}