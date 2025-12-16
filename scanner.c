#include "cig.h"

#include <ctype.h>
#include <string.h>
#include <errno.h>

scanner_t make_scanner(const char *name, const char *buffer, allocator_t allocator) {
	scanner_t s = {
		.name = name,
		.start = buffer,
		.cur = buffer,
		.value = {0},
		.errors = NULL,
		.allocator = allocator
	};
	return s;
}

void scanner_recover(scanner_t *s) {
	while (*s->cur && *s->cur != '\n') {
		s->cur++;
	}
	if (*s->cur == '\n')
		s->cur++;  // move *past* the newline
}

void scanner_error(scanner_t *s, const char *message) {
	const char *error_pos = s->cur; // use current scanner position
	// Compute line + column (1-based)
	int line = 1, col = 1;
	for (const char *p = s->start; p < error_pos; p++) {
		if (*p == '\n') { line++; col = 1; }
		else col++;
	}
	// need 40 bytes for pair of largest possible 64 bit values, then some for
	// ':' and ' ' characters. Just rounded up to 64 because reasons.
	size_t length = strlen(s->name) + 1 + strlen(message) + 1 + 64;
	char *buf = allocator_alloc(s->allocator, length);
	snprintf(
		buf,
		length,
		"%s:%d:%d: %s",
		s->name, line, col, message
	);
	error_node_t *node = allocator_alloc(s->allocator, sizeof(error_node_t));
	node->msg = buf;
	node->next = NULL;
	if (!s->errors) {
		s->errors = node;
	} else {
		error_node_t *p = s->errors;
		while (p->next) p = p->next;
		p->next = node;
	}
}

void scanner_error_and_recover(scanner_t *s, const char *message) {
	scanner_error(s, message);
	scanner_recover(s);
}

bool scan_eof(scanner_t *s) {
	return *s->cur == '\0';
}

bool scan_literal(scanner_t *s, const char *lit) {
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

int scan_repeat_literal(scanner_t *s, const char *lit) {
	int n_repeats = 0;
	while (scan_literal(s, lit)) {
		n_repeats++;
	}
	return n_repeats;
}

bool scan_whitespace(scanner_t *s) {
	const char *save = s->cur;
	while (isspace((unsigned char)*s->cur)) s->cur++;
	return save != s->cur;
}

bool scan_digit(scanner_t *s) {
	if (!isdigit((unsigned char)*s->cur)) {
		return false;
	}
	s->value.digit = (*s->cur) - '0';
	s->cur++;
	return true;
}

bool scan_i64(scanner_t *s) {
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
		scanner_error(s, "integer does not fit in i64 value");
		return false;
	}
	s->cur = end;
	s->value.i64 = val;
	return true;
}

bool scan_i32(scanner_t *s) {
	if (!scan_i64(s)) {
		return false;
	}
	int32_t val = (int32_t)s->value.i64;
	int64_t back = (int64_t)val;
	if (back != s->value.i64) {
		scanner_error(s, "int does not fit in i32 value");
		return false;
	}
	s->value.i32 = val;
	return true;
}

bool scan_i16(scanner_t *s) {
	if (!scan_i64(s)) {
		return false;
	}
	int16_t val = (int16_t)s->value.i64;
	int64_t back = (int64_t)val;
	if (back != s->value.i64) {
		scanner_error(s, "int does not fit in i16 value");
		return false;
	}
	s->value.i16 = val;
	return true;
}

bool scan_i8(scanner_t *s) {
	if (!scan_i64(s)) {
		return false;
	}
	int8_t val = (int8_t)s->value.i64;
	int64_t back = (int64_t)val;
	if (back != s->value.i64) {
		scanner_error(s, "int does not fit in i8 value");
		return false;
	}
	s->value.i8 = val;
	return true;
}

bool scan_u64(scanner_t *s) {
	const char *save = s->cur;
	if (*s->cur == '-') {
		scanner_error(s, "- is not allowed for unsigned integers");
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
		scanner_error(s, "integer does not fit in u64 value");
		return false;
	}
	s->cur = end;
	s->value.u64 = val;
	return true;
}

bool scan_u32(scanner_t *s) {
	if (!scan_u64(s)) {
		return false;
	}
	uint32_t val = (uint32_t)s->value.u64;
	uint64_t back = (uint64_t)val;
	if (back != s->value.u64) {
		scanner_error(s, "int does not fit in u32 value");
		return false;
	}
	s->value.u32 = val;
	return true;
}

bool scan_u16(scanner_t *s) {
	if (!scan_u64(s)) {
		return false;
	}
	uint16_t val = (uint16_t)s->value.u64;
	uint64_t back = (uint64_t)val;
	if (back != s->value.u64) {
		scanner_error(s, "int does not fit in u16 value");
		return false;
	}
	s->value.u16 = val;
	return true;
}

bool scan_u8(scanner_t *s) {
	if (!scan_u64(s)) {
		return false;
	}
	uint8_t val = (uint8_t)s->value.u64;
	uint64_t back = (uint64_t)val;
	if (back != s->value.u64) {
		scanner_error(s, "int does not fit in u8 value");
		return false;
	}
	s->value.u8 = val;
	return true;
}

bool scan_f64(scanner_t *s) {
	const char *save = s->cur;
	char *end;
	errno = 0;
	double val = strtod(s->cur, &end);
	if (end == s->cur) {
		s->cur = save;
		return false;
	}
	if (errno == ERANGE) {
		scanner_error(s, "float does not fit in f64 value");
		return false;
	}
	s->cur = end;
	s->value.f64 = val;
	return true;
}

bool scan_f32(scanner_t *s) {
	if (!scan_f64(s)) {
		return false;
	}
	float val = (float)s->value.f64;
	double back = (double)val;
	if (back != s->value.f64) {
		scanner_error(s, "float does not fit in f32 value");
		return false;
	}
	s->value.f32 = val;
	return true;
}

bool scan_identifier(scanner_t *s) {
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


bool scan_string_literal(scanner_t *s) {
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
					scanner_error(s, "invalid escape sequence");
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
		scanner_error(s, "unterminated string literal");
		s->cur = save;
		return false;
	}
	s->cur++; // skip closing quote
	buf[len] = '\0';
	s->value.string_literal = buf;
	return true;
}

bool scanner_print_errors(scanner_t *s, FILE *fp) {
	for (error_node_t *node = s->errors; node != NULL; node = node->next) {
		fprintf(fp, "%s\n", node->msg);
	}
	return s->errors != NULL;
}

bool looks_like_float(scanner_t *s) {
	const char *cur = s->cur;
	if (*cur == '+' || *cur == '-') cur++;
	if (!isdigit((unsigned char)*(cur++))) return false;
	while (isdigit((unsigned char)*(cur++)));
	if (*cur != '.') return false;
	cur++;
	return isdigit((unsigned char)*cur);
}

bool looks_like_int(scanner_t *s) {
	const char *cur = s->cur;
	if (*cur == '+' || *cur == '-') {
		cur++;
	}
	return isdigit(*cur);
}
