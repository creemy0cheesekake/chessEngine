<img src="https://cdn.shopify.com/s/files/1/0655/2633/3707/files/titan-typhon-mythology_480x480.jpg?v=1728911495" alt="Typhon" style="border-radius: 99999999px; width: 200px; display: block; margin: 16px auto;">

# Typhon

A tournament-strength chess engine written in C++20 with bitboard architecture, alpha-beta search, and a real-time ImGui GUI.

**C++20 • Bitboards • Alpha-Beta Search • ImGui • Transposition Tables**

## Features

### Search & Evaluation

-   **Alpha-beta pruning** with iterative deepening
-   **Quiescence search** to eliminate the horizon effect
-   **Late Move Reductions (LMR)** — reduces search depth for late moves to improve move ordering
-   **MVV-LVA scoring** for capture ordering (Most Valuable Victim, Least Valuable Aggressor)
-   **Killer move heuristics** — 2 killer moves per ply for move ordering
-   **Principal Variation (PV) extraction** for best-line output

### Data Structures

-   **Bitboard representation** using `uint64_t` for all pieces
-   **Zobrist hashing** with incremental hash updates
-   **Transposition table** (16 MB) with 16-byte aligned entries to fit in cache lines and 16-bit partial hash verification
-   **Precomputed attack tables** for knights, kings, and sliding pieces

### Engine Rules

-   Full legal move generation (castling, en passant, promotion, disambiguation)
-   Check, checkmate, and stalemate detection
-   Draw detection: 50-move rule and insufficient material
-   Time-based search with iterative deepening cutoff

### Evaluation

-   Material counting: Q=900, R=500, B=310, N=300, P=100
-   Piece-square tables (PSQT) for positional evaluation
-   Mobility scoring for all sliding and non-sliding pieces
-   King safety in opening/middlegame

## Quick Start

```bash
make        # Build the engine
make run    # Launch the GUI
make test   # Run the doctest suite
```

## Architecture

```
Board (bitboards) → MoveGen (legal moves) → Eval.search()
                                                 ↓
                                         Transposition Table
                                                 ↓
                                         Alpha-Beta Pruning
                                                 ↓
                                         Quiescence Search
                                                 ↓
                                         Best Move
```

## Performance

Moves per second test on the [famous critical position from Kasparov - Topalov (Linares 1999)](https://lichess.org/editor?fen=b2r3r%2Fk4p1p%2Fp2q1np1%2FNp1P4%2F3p1Q2%2FP4PPB%2F1PP4P%2F1K2R3+w+-+-+0+25&variant=standard&color=white):

Average of 10 results:
| Moves | Time (ms) | Moves per second |
| ------- | --------- | ---------------- |
| 4500000 | 652.179 | 6,899,946 |

Results generated using:

```cpp
long long total_moves = 0;
for (int i = 0; i < 100000; ++i) {
    total_moves += b.moveGenerator.genLegalMoves().size();
}
```

## Project Structure

```
src/
├── board.cpp/hpp                   # Bitboard state & FEN parsing
└── consts.hpp                      # Constants & types
├── eval.cpp/hpp                    # Search & evaluation
├── game.cpp/hpp                    # Game logic
├── gui.cpp/hpp                     # ImGui rendering
├── lookup_tables.cpp/hpp           # Precomputed attacks
├── move.cpp/hpp                    # Move encoding, notation
├── move_gen.cpp/hpp                # Legal move generation
├── transposition_table.cpp/hpp     # Direct-addressing hash table
└── util.cpp/hpp                    # Useful utility functions
├── zobrist.cpp/hpp                 # Zobrist hashing
```

## Design Decisions

**Bitboards over 0x88**: Bitboards enable O(1) piece counts via C++20's &lt;bit&gt; header (std::popcount, std::countr_zero), efficient sliding piece attacks via ray lookup, and cache-friendly memory layout.

**Flat move encoding**: Store from-square, to-square, promotion, and flag bits in a single 32-bit integer for cache-friendly move arrays.

**Direct-addressing transposition table**: Use a simple array with hash-key index selection, avoiding pointer chasing and heap allocation overhead.

**LMR reduction formula**: `log2(movesSearched * depthLeft) / 2` — balances search depth reduction against the risk of missing tactical lines.

## Future Additions

-   **NNUE evaluation** — replace handcrafted PSTs with a trained neural network
-   **Syzygy tablebase** — endgame tablebase lookups for positions up to 7 pieces
-   **Search improvements** — null move pruning, futility pruning, singular extensions
-   **Parallel search** — shared hash table with thread-local search trees
-   **UCI protocol** — command-line interface for integration with Arena/XBoard
-   **Opening book** — precomputed lines for faster engine-vs-engine play
-   **Threefold repetition check** — Implement the threefold repetition rule

## Tech Stack

-   **Language**: C++20
-   **GUI**: Dear ImGui + GLFW + OpenGL
-   **Build**: Makefile (g++)
-   **Testing**: doctest
-   **Images**: stb_image.h

## License

MIT
