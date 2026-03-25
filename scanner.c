#include "cig.h"

#include <ctype.h>
#include <string.h>
#include <errno.h>

scan_t make_scan(const char *name, const char *buffer, allocator_t allocator) {
	scan_t s = {
		.name = name,
		.start = buffer,
		.cur = buffer,
		.allocator = allocator
	};
	return s;
}

void scan_next_line(scan_t *s) {
	while (*s->cur && *s->cur != '\n') {
		s->cur++;
	}
	if (*s->cur == '\n')
		s->cur++;  // move *past* the newline
}

scan_result_t scan_eof(scan_t *s) {
	return *s->cur == '\0';
}

bool scan_literal(scan_t *s, const char *lit) {
	const char *save = s->cur;
	while (*lit && *lit == *s->cur) {
		lit++;
		s->cur++;
	}
	if (*lit == '\0') {
		return true;
	}
	s->cur = save;
	return false;
}

int scan_repeat_literal(scan_t *s, const char *lit) {
	int n_repeats = 0;
	while (scan_literal(s, lit)) {
		n_repeats++;
	}
	return n_repeats;
}

bool scan_whitespace(scan_t *s) {
	const char *save = s->cur;
	while (isspace((unsigned char)*s->cur)) s->cur++;
	return save != s->cur;
}

bool scan_digit(scan_t *s) {
	if (!isdigit((unsigned char)*s->cur)) {
		return false;
	}
	s->value.digit = (*s->cur) - '0';
	s->cur++;
	return true;
}

bool scan_i64(scan_t *s) {
	const char *save = s->cur;
	if (*s->cur == '-' || *s->cur == '+') s->cur++;
	if (!isdigit((unsigned char)*s->cur)) {
		s->cur = save;
		return false;
	}
	s->cur = save;
	char *end;
	errno = 0;
	int64_t val = strtoll(s->cur, &end, 10);
	if (end == s->cur) {
		s->cur = save;
		return false;
	}
	if (errno == ERANGE) {
		scan_error(s, "integer does not fit in i64 value");
		return false;
	}
	s->cur = end;
	s->value.i64 = val;
	return true;
}

bool scan_i32(scan_t *s) {
	if (!scan_i64(s)) {
		return false;
	}
	int32_t val = (int32_t)s->value.i64;
	int64_t back = (int64_t)val;
	if (back != s->value.i64) {
		scan_error(s, "int does not fit in i32 value");
		return false;
	}
	s->value.i32 = val;
	return true;
}

bool scan_i16(scan_t *s) {
	if (!scan_i64(s)) {
		return false;
	}
	int16_t val = (int16_t)s->value.i64;
	int64_t back = (int64_t)val;
	if (back != s->value.i64) {
		scan_error(s, "int does not fit in i16 value");
		return false;
	}
	s->value.i16 = val;
	return true;
}

bool scan_i8(scan_t *s) {
	if (!scan_i64(s)) {
		return false;
	}
	int8_t val = (int8_t)s->value.i64;
	int64_t back = (int64_t)val;
	if (back != s->value.i64) {
		scan_error(s, "int does not fit in i8 value");
		return false;
	}
	s->value.i8 = val;
	return true;
}

bool scan_u64(scan_t *s) {
	const char *save = s->cur;
	if (*s->cur == '-') {
		scan_error(s, "- is not allowed for unsigned integers");
		return false;
	}
	if (*s->cur == '+') s->cur++;
	if (!isdigit((unsigned char)*s->cur)) {
		s->cur = save;
		return false;
	}
	s->cur = save;
	char *end;
	errno = 0;
	uint64_t val = strtoull(s->cur, &end, 10);
	if (end == s->cur) {
		s->cur = save;
		return false;
	}
	if (errno == ERANGE) {
		scan_error(s, "integer does not fit in u64 value");
		return false;
	}
	s->cur = end;
	s->value.u64 = val;
	return true;
}

bool scan_u32(scan_t *s) {
	if (!scan_u64(s)) {
		return false;
	}
	uint32_t val = (uint32_t)s->value.u64;
	uint64_t back = (uint64_t)val;
	if (back != s->value.u64) {
		scan_error(s, "int does not fit in u32 value");
		return false;
	}
	s->value.u32 = val;
	return true;
}

bool scan_u16(scan_t *s) {
	if (!scan_u64(s)) {
		return false;
	}
	uint16_t val = (uint16_t)s->value.u64;
	uint64_t back = (uint64_t)val;
	if (back != s->value.u64) {
		scan_error(s, "int does not fit in u16 value");
		return false;
	}
	s->value.u16 = val;
	return true;
}

bool scan_u8(scan_t *s) {
	if (!scan_u64(s)) {
		return false;
	}
	uint8_t val = (uint8_t)s->value.u64;
	uint64_t back = (uint64_t)val;
	if (back != s->value.u64) {
		scan_error(s, "int does not fit in u8 value");
		return false;
	}
	s->value.u8 = val;
	return true;
}

bool scan_f64(scan_t *s) {
	const char *save = s->cur;
	char *end;
	errno = 0;
	double val = strtod(s->cur, &end);
	if (end == s->cur) {
		s->cur = save;
		return false;
	}
	if (errno == ERANGE) {
		scan_error(s, "float does not fit in f64 value");
		return false;
	}
	s->cur = end;
	s->value.f64 = val;
	return true;
}

bool scan_f32(scan_t *s) {
	if (!scan_f64(s)) {
		return false;
	}
	float val = (float)s->value.f64;
	double back = (double)val;
	if (back != s->value.f64) {
		scan_error(s, "float does not fit in f32 value");
		return false;
	}
	s->value.f32 = val;
	return true;
}

bool scan_identifier(scan_t *s) {
	if (!isalpha((unsigned char)*s->cur) && *s->cur != '_') {
		return false;
	}
	const char *start = s->cur++;
	while (isalnum((unsigned char)*s->cur) || *s->cur == '_') {
		s->cur++;
	}
	size_t len = s->cur - start;
	char *buf = allocator_alloc(s->allocator, len + 1);
	memcpy(buf, start, len);
	buf[len] = '\0';
	s->value.identifier = buf;
	return true;
}


bool scan_string_literal(scan_t *s) {
	const char *save = s->cur;
	if (*s->cur != '"')
		return false;
	s->cur++; // skip opening quote
	char *buf = allocator_alloc(s->allocator, 256);
	size_t cap = 256;
	size_t len = 0;
	while (*s->cur && *s->cur != '"') {
		char c = *s->cur++;
		if (c == '\\') {
			char esc = *s->cur++;
			switch (esc) {
				case 'n': c = '\n'; break;
				case 't': c = '\t'; break;
				case 'r': c = '\r'; break;
				case '\\': c = '\\'; break;
				case '"': c = '"'; break;
				case '0': c = '\0'; break;
				default:
					scan_error(s, "invalid escape sequence");
					s->cur = save;
					return false;
			}
		}
		if (len + 2 >= cap) {
			cap *= 2;
			buf = allocator_resize(s->allocator, buf, cap);
		}
		buf[len++] = c;
	}
	if (*s->cur != '"') {
		scan_error(s, "unterminated string literal");
		s->cur = save;
		return false;
	}
	s->cur++; // skip closing quote
	buf[len] = '\0';
	s->value.string_literal = buf;
	return true;
}

scan_value_t required(scan_result_t res) {
	if (!res.ok) {
		fprintf(stderr, "%s:%d:%d: %s", res.filename, res.line, res.column, res.error_message);
		exit(1);
	}
	return res.value;
	// TODO
}

typedef struct scan_error {
	const char *filename;
	int line;
	int column;
	const char *error_message;
} scan_error_t;
