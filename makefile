
CC := gcc
CFLAGS := -pedantic -Wall -Wextra -Wno-override-init -O0 -g -fno-omit-frame-pointer -fno-inline
LDFLAGS := -lcriterion

TESTBIN := /tmp/all_tests

.PHONY: test

run_test: test
	$(TESTBIN)

run_test_memcheck: test
	valgrind --leak-check=full --show-leak-kinds=all --trace-children=yes --error-exitcode=1 $(TESTBIN); \
	

test:
	@echo "Discovering test files..."
	@files=$$(find . -type f -name 'test*.c'); \
	if [ -z "$$files" ]; then \
		echo "No test files found!"; \
	else \
		echo "Compiling all test files into $(TESTBIN)..."; \
		$(CC) $(CFLAGS) _allocator_impl.c $$files -o $(TESTBIN) $(LDFLAGS) || exit 1; \
	fi

PROJECT_ROOT := $(shell pwd)
PROJECT_C_FILES := $(shell find $(PROJECT_ROOT) \( -name '*.c' -o -name '*.h' \))
CRITERION_HEADERS := $(shell find /usr/include/criterion)
ALL_FILES := $(PROJECT_C_FILES) $(CRITERION_HEADERS)
tags: $(ALL_FILES)
	ctags -R $(ALL_FILES)
