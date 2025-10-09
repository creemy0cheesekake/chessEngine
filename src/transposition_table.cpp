#include "transposition_table.hpp"
#include <iostream>

std::array<TTEntry, TT_SIZE_MB / sizeof(TTEntry)> TranspositionTable::m_table{};
int TranspositionTable::m_used = 0;
int TranspositionTable::m_size = TT_SIZE_MB / sizeof(TTEntry);

void TranspositionTable::add(ZobristHash h, Centipawns s, int d, TTFlag f, Move m) {
	size_t index = h % m_table.size();

	TTEntry newEntry(h, s, d, f, m);

	TTEntry& entry = m_table[index];
	if (entry.depth <= d ||
		(entry.depth == d && (f == EXACT || entry.flag != EXACT)) ||
		entry.partial_hash != (h & 0xFFFF)) {
		entry = newEntry;
	}
}

Move TranspositionTable::getMove(MoveSkeleton m) {
	return Move(m.from, m.to, m.piece, m.promoPiece, m.flags);
}

TTEntry TranspositionTable::getEntry(ZobristHash h) {
	size_t index = h % m_table.size();
	return m_table[index];
}

void TranspositionTable::printCapacity() {
	for (int i = 0; i < m_table.size(); i++) {
		if (m_table[i].partial_hash != 0) {
			m_used++;
		}
	}

	std::cout << "Transposition Table Capacity: " << m_used << " / " << m_size << " = " << m_used / (float)m_size * 100.0f << " %" << std::endl;
}

void TranspositionTable::reset() {
	m_used = 0;
	for (auto& entry : m_table) {
		entry = TTEntry();
	}
}