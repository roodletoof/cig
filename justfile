CC := "zig cc"
CFLAGS := "-pedantic -Wall -Wextra -Wno-override-init -O0 -g -fno-omit-frame-pointer -fno-inline"
LDFLAGS := "-lcriterion"
TESTBIN := "/tmp/all_tests"

set shell := ["bash", "-cu"]

FOOBAR := "foobar"

run_foobar: build_foobar
    ./{{FOOBAR}}

build_foobar:
    {{CC}} ./foobar.c -o {{FOOBAR}} {{CFLAGS}}

clean_foobar:
    rm {{FOOBAR}}

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

tag:
	find "$PWD" -type f \( -name '*.c' -o -name '*.h' \) | xargs ctags
