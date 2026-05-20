CC := "zig cc"
CFLAGS := "-pedantic -Wall -Wextra -Wno-override-init -O0 -g -fno-omit-frame-pointer -fno-inline"
LDFLAGS := if os() == "macos" {
    "$(pkg-config --libs --cflags criterion)"
} else {
    "-lcriterion"
}

TMP := "/tmp"

TESTBIN := TMP/"all_tests"

set shell := ["bash", "-cu"]


FOOBAR := "foobar"
FOOBAR_SOURCE := FOOBAR+".c"
FOOBAR_BIN := TMP/FOOBAR
FOOBAR_EXPANDED := TMP/FOOBAR+"-expanded.c"

build_foobar2:
    {{CC}} {{FOOBAR_SOURCE}} {{CFLAGS}} -o foobar2

run_foobar: build_foobar
    {{FOOBAR_BIN}}

build_foobar: foobar_expanded
    {{CC}} {{FOOBAR_EXPANDED}} {{CFLAGS}} -o {{FOOBAR_BIN}}
    echo "{{FOOBAR_BIN}}"

foobar_expanded:
    {{CC}} -P -E {{FOOBAR_SOURCE}} -o {{FOOBAR_EXPANDED}}
    clang-format -i {{FOOBAR_EXPANDED}}

clean_foobar:
    rm {{FOOBAR_BIN}}

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
} else {
    "TODO"
}

tag:
    echo "Making tags from project and headers in {{DEPENDENCY_HEADERS}}"
    ctags --languages=c --langmap=c:.c.h -R {{DEPENDENCY_HEADERS}} .
