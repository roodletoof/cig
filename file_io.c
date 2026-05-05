#include "cig.h"
#include <assert.h>

char *read_entire_file(const char *path, allocator_t allocator) {
	FILE *fp = fopen(path, "rb");
	assert(fp != NULL);
	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	rewind(fp);
	char *buf = allocator_alloc(allocator, size+1);
	assert(buf != NULL);
	size_t read = fread(buf, 1, size, fp);
	assert(read == (size_t)size);
	fclose(fp);
	buf[size] = '\0';
	return buf;
}

// Returns dynamic array, of fixed char strings.
char **read_all_file_lines(const char *path, allocator_t allocator) {
	char *contents = read_entire_file(path, allocator);
	char **lines = arr_make(char*, allocator);
	arr_append(lines, contents);
	bool just_split = false;
	for (char *c = contents; (*c)!='\0'; c++) {
		if ((*c) == '\n') {
			*c = '\0';
			just_split = true;
			continue;
		}
		if (just_split) {
			just_split = false;
			arr_append(lines, c);
		}
	}
	return lines;
}
