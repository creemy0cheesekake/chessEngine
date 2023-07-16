#ifndef UTIL
#define UTIL

#include <bitset>
#include "consts.hpp"

bool inRange(int val, int start, int end) {
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

bitboard MS1B(bitboard x) {
	x |= x >> 32;
	x |= x >> 16;
	x |= x >> 8;
	x |= x >> 4;
	x |= x >> 2;
	x |= x >> 1;
	return (x >> 1) + 1;
}

bitboard LS1B(bitboard x) {
	return x & -x;
}

#endif
