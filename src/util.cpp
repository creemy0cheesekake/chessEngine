#ifndef UTIL
#define UTIL

#include <bitset>
#include <chrono>
#include <iostream>

#include "consts.hpp"

inline bool inRange(int val, int start, int end) {
	return val >= start && val <= end;
}

auto bin(long num) {
	return std::bitset<64>(num);
}

std::string bboard(bitboard b) {
	std::string board = "";
	int i			  = 56;
	while (i >= 0 and i < 64) {
		char piece = '.';
		if (1UL << i & b) piece = '1';
		board = (board + piece) + ' ';
		if (++i % 8 == 0) {
			i -= 16;
			board += '\n';
		}
	}
	return board;
}

inline bitboard MS1B(bitboard x) {
	x |= x >> 32;
	x |= x >> 16;
	x |= x >> 8;
	x |= x >> 4;
	x |= x >> 2;
	x |= x >> 1;
	return (x >> 1) + 1;
}

inline bitboard LS1B(bitboard x) {
	return x & -x;
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
	// std::ostream &os = std::cout;
	std::ostream &os = std::cout;
	os << d << " " << unit << std::endl;
}
void end() {
	endTime	 = std::chrono::high_resolution_clock::now();
	duration = endTime - startTime;
	format(duration.count());
}
}

#endif
