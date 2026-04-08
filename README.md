# ChessEngine2

A tournament-strength chess engine written in C++20 with bitboard architecture, alpha-beta search, and a real-time ImGui GUI.

```
C++20 • Bitboards • Alpha-Beta Search • ImGui • Transposition Tables
```

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
-   **Transposition table** (16 MB) with 16-byte aligned entries and 16-bit partial hash verification
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

Perft results (positions generated) on the [Kiwipete test position](https://www.chessprogramming.org/Perft_Results#Position_2):

| Depth | Nodes  | Time (ms) |
| ----- | ------ | --------- |
| 1     | 48     | TBD       |
| 2     | 2,039  | TBD       |
| 3     | 97,862 | TBD       |

## Project Structure

```
src/
├── board.cpp/hpp          # Bitboard state & FEN parsing
├── move.cpp/hpp           # Move encoding, notation
├── move_gen.cpp/hpp       # Legal move generation
├── eval.cpp/hpp           # Search & evaluation
├── transposition_table.cpp/hpp  # Hash table
├── zobrist.cpp/hpp        # Zobrist hashing
├── lookup_tables.cpp/hpp  # Precomputed attacks
├── gui.cpp/hpp            # ImGui rendering
└── consts.hpp             # Constants & types
```

## Design Decisions

**Bitboards over 0x88**: Bitboards enable O(1) piece counts with `__builtin_popcountll`, efficient sliding piece attacks via ray lookup, and cache-friendly memory layout.

**Full BoardState copies**: Chose simplicity over micro-optimization. On undo, we restore the full board state rather than incrementally reverting changes. This eliminates a class of bugs at a small memory cost.

**LMR reduction formula**: `log2(movesSearched * depthLeft) / 2` — balances search depth reduction against the risk of missing tactical lines.

## Tech Stack

-   **Language**: C++20
-   **GUI**: Dear ImGui + GLFW + OpenGL
-   **Build**: Makefile (g++)
-   **Testing**: doctest
-   **Images**: stb_image.h

## License

MIT
