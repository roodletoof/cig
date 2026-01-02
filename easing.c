#include "cig.h"

static float clamp(float t, float min, float max) {
	if (t < min) {
		return min;
	}
	if (max < t) {
		return max;
	}
	return t;
}

static float norm(float t) {
	return clamp(t, 0.0f, 1.0f);
}

float ease_in_quad(float t) {
	t = norm(t);
	return t * t;
}

float ease_out_quad(float t) {
}

float ease_inout_quad(float t) {
}

float ease_in_cubic(float t) {
}

float ease_out_cubic(float t) {
}

float ease_inout_cubic(float t) {
}

float ease_in_quart(float t) {
}

float ease_out_quart(float t) {
}

float ease_inout_quart(float t) {
}

float ease_in_quint(float t) {
}

float ease_out_quint(float t) {
}

float ease_inout_quint(float t) {
}

float ease_in_expo(float t) {
}

float ease_out_expo(float t) {
}

float ease_inout_expo(float t) {
}

float ease_in_circ(float t) {
}

float ease_out_circ(float t) {
}

float ease_inout_circ(float t) {
}

float ease_in_back(float t) {
}

float ease_out_back(float t) {
}

float ease_inout_back(float t) {
}

float ease_in_elastic(float t) {
}

float ease_out_elastic(float t) {
}

float ease_inout_elastic(float t) {
}

float ease_in_bounce(float t) {
}

float ease_out_bounce(float t) {
}

float ease_inout_bounce(float t) {
}
