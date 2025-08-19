#ifndef UTIL_H
#define UTIL_H

#include <bitset>
#include <chrono>

#include "consts.hpp"

/**
* @brief returns whether or not val is between start and end inclusive
*/
inline bool inRange(int val, int start, int end) {
	return val >= start && val <= end;
}

/**
* @brief returns a 64 bit representation of num
*/
inline auto bin(long num) {
	return std::bitset<64>(num);
}

/**
* @brief returns the most significant 1 bit from the input. essentially the bit closest to h8 in little endian rook file.
* __builtin_clzll is a gcc builtin function that counts the number of leading zeros
*/
inline Bitboard MS1B(Bitboard x) {
	return (1UL << h8) >> __builtin_clzll(x);
}

/**
* @brief returns the least significant 1 bit from the input. essentially the bit closest to a1 in little endian rook file
*/
inline Bitboard LS1B(Bitboard x) {
	return x & -x;
}

/**
* @brief removes the least significant 1 bit from the input and returns the input
*/
inline Bitboard removeLS1B(Bitboard& x) {
	x &= x - 1;
	return x;
}

/**
* @brief returns an ascii representation of a bitboard shaped in an 8x8 grid. 1 if the corresponding bit for that square is a 1, and a '.' otherwise
*/
std::string bboard(Bitboard b);

namespace Timer {
/**
* @brief holds the starting and ending times of the timer
*/
extern std::chrono::time_point<std::chrono::high_resolution_clock> startTime, endTime;

/**
* @brief holds the duration of the timer. basically just the difference between endTime and startTime
*/
extern std::chrono::duration<float> duration;

/**
* @brief starts the timer, sets startTime
*/
void start();

/**
* @brief ends the timer, sets endTime
*/
void end();

/**
* @brief formats the time with human readable units, using seconds, milliseconds and microseconds
*/
void format(float d);
}

#endif
