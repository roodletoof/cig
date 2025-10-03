# Makefile

# Compiler and flags
CC := gcc
CFLAGS := -Wall -Wextra -O2
LDFLAGS := -lcriterion

# Output binary
TESTBIN := /tmp/all_tests

# Phony target
.PHONY: test

test:
	@echo "Discovering test files..."
	@files=$$(find . -type f -name 'test*.c'); \
	if [ -z "$$files" ]; then \
		echo "No test files found!"; \
	else \
		echo "Compiling all test files into $(TESTBIN)..."; \
		$(CC) $(CFLAGS) $$files -o $(TESTBIN) $(LDFLAGS); \
		echo "Running tests..."; \
		$(TESTBIN); \
	fi
