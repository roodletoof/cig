#ifndef CLI_HEADER
#define CLI_HEADER

#include <stdbool.h>

typedef struct args {
    int count;
    char **values;
} args_t;

args_t make_args(int arg_count, char **args);
bool command(args_t *args, char *command_name);
bool parse_bool(args_t args, char *flag_name);
char *parse_string(args_t args, char *flag_name);
int parse_int(args_t args, char *flag_name);


#ifdef CLI_IMPL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


bool command(args_t *args, char *command_name) {
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

bool parse_bool(args_t args, char *flag_name) {
    for ( int i = 0; i < args.count; i++ ) {
        if (strcmp(args.values[i], flag_name) == 0) {
            return true;
        }
    }
    return false;
}

char *parse_string(args_t args, char *flag_name) {
    bool is_found = false;
    int flag_name_index;
    for ( int i = 0; i < args.count; i++ ) {
        if (strcmp(args.values[i], flag_name) == 0) {
            is_found = true;
            flag_name_index = i;
            break;
        }
    }
    if (!is_found) {
        fprintf(stderr, "Arg %s not found!\n", flag_name);
        exit(1);
    }
    int flag_value_index = flag_name_index + 1;
    if (flag_value_index >= args.count) {
        fprintf(stderr, "No value provided for %s!\n", flag_name);
        exit(1);
    }
    return args.values[flag_value_index];
}

int parse_int(args_t args, char *flag_name) {
    char *text = parse_string(args, flag_name);
    for ( char *c = text; (*c) != '\0'; c++ ) {
        if ((*c) < '0' || '9' < (*c)) {
            fprintf(stderr, "Invalid integer %s!\n", text);
            exit(1);
        }
    }
    int result = atoi(text);
    return result;
}

args_t make_args(int arg_count, char **args) {
    return (args_t) {
        .count = arg_count-1,
        .values = args+1,
    };
}

#endif /* CLI_IMPL */
#endif /* CLI_HEADER */
