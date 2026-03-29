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

// TODO: probably do not need an allocator for a scanner?

scan_error_t scan_error(const scan_t *s, const char *message) {
	const char *error_pos = s->cur; // use current scanner position
	// Compute line + column (1-based)
	int line = 1, col = 1;
	for (const char *p = s->start; p < error_pos; p++) {
		if (*p == '\n') { line++; col = 1; }
		else col++;
	}

	return (scan_error_t) {
		.filename = s->name,
		.line = line,
		.column = col,
		.error_message = message
	};

}

#define scan_value_result(VAL) ((scan_result_t){.ok=true, .value=(scan_value_t){VAL}})

scan_result_t scan_error_result(const scan_t *s, const char *message) {
	return (scan_result_t) {
		.ok=false,
		.error=scan_error(s, message),
	};
}

scan_result_t scan_eof(scan_t *s) {
	return (scan_result_t) {
		.ok=*s->cur == '\0',
		.error=scan_error(s, "null character not found"),
	};
}

scan_result_t scan_literal(scan_t *s, const char *lit) {
	const char *save = s->cur;
	while (*lit && *lit == *s->cur) {
		lit++;
		s->cur++;
	}
	if (*lit == '\0') {
		return scan_value_result();
	}
	s->cur = save;
	return scan_error_result(s, "invalid literal");
	// TODO: add another optional string field to specify what literal is
	// missing
}

int scan_repeat_literal(scan_t *s, const char *lit) {
	int n_repeats = 0;
	while (scan_literal(s, lit).ok) {
		n_repeats++;
	}
	return n_repeats;
}

scan_result_t scan_whitespace(scan_t *s) {
	const char *save = s->cur;
	while (isspace((unsigned char)*s->cur)) s->cur++;
	return (scan_result_t) {
		.ok=save != s->cur,
		.error=scan_error(s, "expected whitespace"),
	};
}

scan_result_t scan_digit(scan_t *s) {
	if (!isdigit((unsigned char)*s->cur)) {
		return scan_error_result(s, "expected digit");
	}
	s->cur++;
	return scan_value_result(.digit=(*s->cur) - '0');
}

scan_result_t scan_i64(scan_t *s) {
	const char *save = s->cur;
	if (*s->cur == '-' || *s->cur == '+') s->cur++;
	if (!isdigit((unsigned char)*s->cur)) {
		s->cur = save;
		return scan_error_result(s, "is not an integer");
	}
	s->cur = save;
	char *end;
	errno = 0;
	int64_t val = strtoll(s->cur, &end, 10);
	if (end == s->cur) {
		s->cur = save;
		return scan_error_result(s, "is not an integer");
	}
	if (errno == ERANGE) {
		return scan_error_result(s, "integer does not fit in i64 value");
	}
	s->cur = end;
	return (scan_result_t){
		.ok=true,
		.value=(scan_value_t){
			.i64=val,
		},
	};
}

scan_result_t scan_i32(scan_t *s) {
	scan_result_t result_i64 = scan_i64(s);
	if (!result_i64.ok) {
		return result_i64;
	}
	int32_t val = (int32_t)result_i64.value.i64;
	int64_t back = (int64_t)val;
	if (back != result_i64.value.i64) {
		return scan_error_result(s, "integer does not fit in i32 value");
	}
	return (scan_result_t) {
		.ok=true,
		.value=(scan_value_t){
			.i32=val,
		},
	};
}

scan_result_t scan_i16(scan_t *s) {
	scan_result_t result_i64 = scan_i64(s);
	if (!result_i64.ok) {
		return result_i64;
	}
	int16_t val = (int16_t)result_i64.value.i64;
	int64_t back = (int64_t)val;
	if (back != result_i64.value.i64) {
		return scan_error_result(s, "integer does not fit in i16 value");
	}
	return (scan_result_t) {
		.ok=true,
		.value=(scan_value_t){
			.i16=val,
		},
	};
}

scan_result_t scan_i8(scan_t *s) {
	scan_result_t result_i64 = scan_i64(s);
	if (!result_i64.ok) {
		return result_i64;
	}
	int8_t val = (int8_t)result_i64.value.i64;
	int64_t back = (int64_t)val;
	if (back != result_i64.value.i64) {
		return scan_error_result(s, "integer does not fit in i8 value");
	}
	return (scan_result_t) {
		.ok=true,
		.value=(scan_value_t){
			.i8=val,
		},
	};
}

scan_result_t scan_u64(scan_t *s) {
	const char *save = s->cur;
	if (*s->cur == '-') {
		return scan_error_result(s, "- is not allowed for unsigned integers");
	}
	if (*s->cur == '+') s->cur++;
	if (!isdigit((unsigned char)*s->cur)) {
		s->cur = save;
		return scan_error_result(s, "not an integer");
	}
	s->cur = save;
	char *end;
	errno = 0;
	uint64_t val = strtoull(s->cur, &end, 10);
	if (end == s->cur) {
		s->cur = save;
		return scan_error_result(s, "not an integer");
	}
	if (errno == ERANGE) {
		return scan_error_result(s, "integer does not fit in i64 value");
	}
	s->cur = end;
	return (scan_result_t){
		.ok=true,
		.value=(scan_value_t){
			.u64=val,
		},
	};
}

scan_result_t scan_u32(scan_t *s) {
	scan_result_t result_u64 = scan_u64(s);
	if (!result_u64.ok) {
		return result_u64;
	}
	uint32_t val = (uint32_t)result_u64.value.u64;
	uint64_t back = (uint64_t)val;
	if (back != result_u64.value.u64) {
		return scan_error_result(s, "integer does not fit in u32 values");
	}
	return (scan_result_t) {
		.ok=true,
		.value=(scan_value_t){
			.u32=val,
		},
	};
}

scan_result_t scan_u16(scan_t *s) {
	scan_result_t result_u64 = scan_u64(s);
	if (!result_u64.ok) {
		return result_u64;
	}
	uint16_t val = (uint16_t)result_u64.value.u64;
	uint64_t back = (uint64_t)val;
	if (back != result_u64.value.u64) {
		return scan_error_result(s, "integer does not fit in u16 value");
	}
	return (scan_result_t) {
		.ok=true,
		.value=(scan_value_t){
			.u16=val,
		},
	};
}

scan_result_t scan_u8(scan_t *s) {
	scan_result_t result_u64 = scan_u64(s);
	if (!result_u64.ok) {
		return result_u64;
	}
	uint8_t val = (uint8_t)result_u64.value.u64;
	uint64_t back = (uint64_t)val;
	if (back != result_u64.value.u64) {
		return scan_error_result(s, "integer does not fit in u8 value");
	}
	return (scan_result_t) {
		.ok=true,
		.value=(scan_value_t){
			.u8=val,
		},
	};
}

scan_result_t scan_f64(scan_t *s) {
	const char *save = s->cur;
	char *end;
	errno = 0;
	double val = strtod(s->cur, &end);
	if (end == s->cur) {
		s->cur = save;
		return scan_error_result(s, "not a float");
	}
	if (errno == ERANGE) {
		return scan_error_result(s, "float does not fit in f64 value");
	}
	s->cur = end;
	return (scan_result_t) {
		.ok=true,
		.value=(scan_value_t){
			.f64=val,
		},
	};
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
