CC := "zig cc"
CFLAGS := "-pedantic -Wall -Wextra -Wno-override-init -O0 -g -fno-omit-frame-pointer -fno-inline -std=c99"
LDFLAGS := "-lcriterion"
TESTBIN := "/tmp/all_tests"

set shell := ["bash", "-cu"]

test: build
	{{TESTBIN}}

test_memcheck: build
	valgrind --leak-check=full \
	          --show-leak-kinds=all \
	          --trace-children=yes \
	          --error-exitcode=1 \
	          {{TESTBIN}}

build:
	find "$PWD" -type f -name 'test*.c' | xargs -r {{CC}} {{CFLAGS}} _allocator_impl.c -o {{TESTBIN}} {{LDFLAGS}}

tags:
	find "$PWD" -type f \( -name '*.c' -o -name '*.h' \) | xargs ctags
