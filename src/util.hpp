#ifndef UTIL_H
#define UTIL_H

#include <bitset>
#include <chrono>

#include "consts.hpp"

inline bool inRange(int val, int start, int end) {
	return val >= start && val <= end;
}

inline auto bin(long num) {
	return std::bitset<64>(num);
}

inline bitboard MS1B(bitboard x) {
	return 0x8000000000000000ULL >> __builtin_clzll(x);
}

inline bitboard LS1B(bitboard x) {
	return x & -x;
}

std::string bboard(bitboard b);
float round(float n, int numOfDigits);

namespace Timer {
extern std::chrono::time_point<std::chrono::high_resolution_clock> startTime, endTime;
extern std::chrono::duration<float> duration;

void start();
void format(float d);
void end();
}

#endif
