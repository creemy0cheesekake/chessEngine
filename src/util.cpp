#include <bitset>

bool inRange(int val, int start, int end) {
	return val >= start && val <= end;
}

auto bin(long num) {
	return std::bitset<64>(num);
}
