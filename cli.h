#ifndef CLI_HEADER
#define CLI_HEADER

#include <stdbool.h>

#define CLI_UNIQUE1 __macro_internal_34bba35b8b9b20a75f9881e3795630e25d36e620d9c9741e2e9141ba82ec6ef7__
#define CLI_UNIQUE2 __macro_internal_34bba35b8b9b20a75f9881e3795630e25d36e620d9c9741e2e9141ba82ec6ef8__

typedef struct args {
    int count;
    const char **values;
} args_t;

args_t cli_make_args(int argc, const char **argv); // no crash
bool cli_command(args_t *args, const char *command_name); // no crash
bool cli_bool(args_t args, const char *flag_name); // no crash
bool cli_opt_str_func(args_t args, const char *flag_name, const char **output, const char *file, int line);
const char *cli_req_str_func(args_t args, const char *flag_name, const char *file, int line);
bool cli_opt_int_func(args_t args, const char *flag_name, int *output, const char *file, int line);
int cli_req_int_func(args_t args, const char *flag_name, const char *file, int line);

#define cli_opt_str(ARGS, FLAG_NAME, OUTPUT) cli_opt_str_func(ARGS, FLAG_NAME, OUTPUT, __FILE__, __LINE__)
#define cli_req_str(ARGS, FLAG_NAME) cli_req_str_func(ARGS, FLAG_NAME, __FILE__, __LINE__)
#define cli_opt_int(ARGS, FLAG_NAME, OUTPUT) cli_opt_int_func(ARGS, FLAG_NAME, OUTPUT, __FILE__, __LINE__)
#define cli_req_int(ARGS, FLAG_NAME) cli_req_int_func(ARGS, FLAG_NAME, __FILE__, __LINE__)

#define with_opt_str(ARGS, FLAG_NAME, VAR) \
    for (const char *VAR = NULL; VAR == NULL;) \
    for (bool CLI_UNIQUE1 = true; CLI_UNIQUE1;) \
    for ( \
        bool CLI_UNIQUE2 = (CLI_UNIQUE1 = cli_opt_str(ARGS, FLAG_NAME, &VAR), true); \
        CLI_UNIQUE2; \
        CLI_UNIQUE2 = (VAR = ((const char *) 1), CLI_UNIQUE1 = false, false) \
    ) \
    if (CLI_UNIQUE1)

#define with_opt_int(ARGS, FLAG_NAME, VAR) \
    for (int VAR = 0; VAR == 0;) \
    for (bool CLI_UNIQUE1 = true; CLI_UNIQUE1;) \
    for ( \
        bool CLI_UNIQUE2 = (CLI_UNIQUE1 = cli_opt_int(ARGS, FLAG_NAME, &VAR), true); \
        CLI_UNIQUE2; \
        CLI_UNIQUE2 = (VAR = 1, CLI_UNIQUE1 = false, false) \
    ) \
    if (CLI_UNIQUE1)

#ifdef CLI_IMPL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool cli_command(args_t *args, const char *command_name) {
    if (args->count == 0) {
        return false;
    }
    if (strcmp(args->values[0], command_name) == 0) {
        args->values++;
        args->count--;
        return true;
    }
    return false;
}

bool cli_bool(args_t args, const char *flag_name) {
    for ( int i = 0; i < args.count; i++ ) {
        if (strcmp(args.values[i], flag_name) == 0) {
            return true;
        }
    }
    return false;
}

bool cli_opt_str_func(args_t args, const char *flag_name, const char **output, const char *file, int line) {
    bool is_flag_name_found = false;
    int flag_name_index;
    for ( int i = 0; i < args.count; i++ ) {
        if (strcmp(args.values[i], flag_name) == 0) {
            is_flag_name_found = true;
            flag_name_index = i;
            break;
        }
    }
    static const char *dummy = "";
    if (!is_flag_name_found) {
        *output = dummy;
        return false;
    }
    int flag_value_index = flag_name_index + 1;
    if (args.count <= flag_value_index) {
        fprintf(stderr, "%s:%d: No value provided for %s!\n", file, line, flag_name);
        exit(1);
    }
    *output = args.values[flag_value_index];
    return true;
}

const char *cli_req_str_func(args_t args, const char *flag_name, const char *file, int line) {
    const char *value;
    if (cli_opt_str_func(args, flag_name, &value, file, line)) {
        return value;
    } else {
        fprintf(stderr, "%s:%d: Required string %s not provided!\n", file, line, flag_name);
        exit(1);
    }
    return "";
}

bool cli_opt_int_func(args_t args, const char *flag_name, int *output, const char *file, int line) {
    const char *text = "";
    if (cli_opt_str_func(args, flag_name, &text, file, line)) {
        if (
            text[0] != '-' &&
            text[0] < '0' &&
            '9' < text[0]
        ) {
            fprintf(stderr, "%s:%d: Invalid integer %s!\n", file, line, text);
            exit(1);
        }
        if (
            text[0] == '-' &&
            text[1] == '\0'
        ) {
            fprintf(stderr, "%s:%d: Invalid integer %s!\n", file, line, text);
            exit(1);
        }
        for ( const char *c = &text[1]; (*c) != '\0'; c++ ) {
            if ((*c) < '0' || '9' < (*c)) {
                fprintf(stderr, "%s:%d: Invalid integer %s!\n", file, line, text);
                exit(1);
            }
        }
        *output = atoi(text);
        return true;
    }
    *output = 0;
    return false;
}

int cli_req_int_func(args_t args, const char *flag_name, const char *file, int line) {
    int value;
    if (cli_opt_int_func(args, flag_name, &value, file, line)) {
        return value;
    } else {
        fprintf(stderr, "%s:%d: Required int %s not provided!\n", file, line, flag_name);
        exit(1);
    }
    return 0;
}

args_t cli_make_args(int argc, const char **argv) {
    return (args_t) {
        .count = argc-1,
        .values = argv+1,
    };
}

#endif /* CLI_IMPL */
#endif /* CLI_HEADER */
