CC := "zig cc"
CFLAGS := "-pedantic -Wall -Wextra -Wno-override-init -O0 -g3 -fno-omit-frame-pointer -fno-inline -fsanitize=address,undefined -fno-sanitize-recover=all"
LDFLAGS := if os() == "macos" {
    "$(pkg-config --libs --cflags criterion)"
} else {
    "-lcriterion"
}

TMP := "/tmp"

TESTBIN := TMP/"all_tests"

set shell := ["bash", "-cu"]

[default]
test: build
    {{TESTBIN}}

test_memcheck: build
    valgrind --leak-check=full \
              --show-leak-kinds=all \
              --trace-children=yes \
              --error-exitcode=1 \
              {{TESTBIN}}

build:
    find . -type f -name 'test*.c' | xargs -r {{CC}} {{CFLAGS}} {{LDFLAGS}} _allocator_impl.c -o {{TESTBIN}}

DEPENDENCY_HEADERS := if os() == "macos" {
    "$(pkg-config --cflags-only-I criterion | sed 's/-I//g')"
} else if os() == "linux" {
    "/usr/include/"
} else {
    "TODO"
}

tag:
    ctags --languages=c --langmap=c:.c.h -R .

tag-all:
    echo "Making tags from project and headers in {{DEPENDENCY_HEADERS}}"
    ctags --languages=c --langmap=c:.c.h -R {{DEPENDENCY_HEADERS}} .
