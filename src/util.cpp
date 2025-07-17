#include "util.hpp"
#include <cmath>
#include <iostream>

std::string bboard(bitboard b) {
	std::string board = "";
	int i			  = 56;
	while (i >= 0 && i < 64) {
		char piece = '.';
		if (1ULL << i & b) {
			piece = '1';
		}
		board = (board + piece) + ' ';
		if (++i % 8 == 0) {
			i -= 16;
			board += '\n';
		}
	}
	return board;
}

float round(float n, int numOfDigits) {
	float value;
	if (n > 0) {
		value = (int)(n * std::pow(10, numOfDigits) + .5);
	} else {
		value = (int)(n * std::pow(10, numOfDigits) - .5);
	}
	return value / std::pow(10, numOfDigits);
}

namespace Timer {
std::chrono::time_point<std::chrono::high_resolution_clock> startTime, endTime;
std::chrono::duration<float> duration;

void start() {
	startTime = std::chrono::high_resolution_clock::now();
}

void format(float d) {
	std::string unit = "s";
	if (d >= 60) {
		d /= 60.0f;
		unit = "m";
	}
	if (d < 1) {
		d *= 1000.0f;
		unit = "ms";
	}
	if (d < 1) {
		d *= 1000.0f;
		unit = "μs";
	}
	std::cout << d << " " << unit << std::endl;
}

void end() {
	endTime	 = std::chrono::high_resolution_clock::now();
	duration = endTime - startTime;
	format(duration.count());
}
}
