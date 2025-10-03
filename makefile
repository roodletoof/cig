
CC := gcc
CFLAGS := -Wall -Wextra -Wno-override-init -O2 -fsanitize=address
LDFLAGS := -lcriterion

TESTBIN := /tmp/all_tests

.PHONY: test

test:
	@echo "Discovering test files..."
	@files=$$(find . -type f -name 'test*.c'); \
	if [ -z "$$files" ]; then \
		echo "No test files found!"; \
	else \
		echo "Compiling all test files into $(TESTBIN)..."; \
		$(CC) $(CFLAGS) _allocator_impl.c $$files -o $(TESTBIN) $(LDFLAGS); \
		echo "Running tests..."; \
		$(TESTBIN); \
	fi
