#include "cig.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void cli_print_indentation(args_t args) {
	for ( int i = 0; i < args.indentation; i++ ) {
		printf("\t");
	}
}

bool cli_command(args_t *args, const char *command_name) {
	if (args->help) {
		cli_print_indentation(*args);
		printf("[ cmd ] %s\n", command_name);
	}
	if (args->count == 0) { return false; }
	if (strcmp(args->values[0], command_name) == 0) {
		args->values++;
		args->count--;
		args->indentation++;
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
	if (args.help) {
		cli_print_indentation(args);
		printf("[ opt ] %s\n", flag_name);
		*output = "";
		return false;
	}
	
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
	if (args.help) {
		cli_print_indentation(args);
		printf("[ req ] %s\n", flag_name);
		return "";
	}
	
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
	if (args.help) {
		cli_print_indentation(args);
		printf("[ opt ] %s\n", flag_name);
		*output = 0;
		return false;
	}
	
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
	if (args.help) {
		cli_print_indentation(args);
		printf("[ req ] %s\n", flag_name);
		return 0;
	}
	
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
	args_t args = (args_t) {
		.count = argc-1,
		.values = argv+1,
		.help = false,
		.indentation = 0,
	};
	args.help = cli_bool(args, "--help") || cli_bool(args, "-h");
	return args;
}

