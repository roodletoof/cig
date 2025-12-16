#include <assert.h>
#include <criterion/criterion.h>
#include "cig.h"

Test(scanner, looks_like_float) {
	const char *buffer =
		"1.0 true\n"
		"12.34 true\n"
		"0.5 true\n"
		"9.99 true\n"
		"+1.2 true\n"
		"-3.14 true\n"
		"1 false\n"
		"42 false\n"
		"+7 false\n"
		"1. false\n"
		".5 false\n"
		"+. false\n"
		"abc false\n"
		"+x false\n";
	with_borrow(allocator) {
		scanner_t scanner = make_scanner("test", buffer, allocator);
		while (!scan_eof(&scanner)) {
			bool actual = looks_like_float(&scanner);
			while (*scanner.cur++ != ' ');
			bool expect;
			if (scan_literal(&scanner, "true")) {
				expect = true;
			} else if (scan_literal(&scanner, "false")) {
				expect = false;
			} else {
				assert(false && "invalid expectation");
			}
			scan_whitespace(&scanner);
			cr_assert_eq(actual, expect);

		}
	}

}
