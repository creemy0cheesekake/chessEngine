#include "../include/doctest.h"
#include "custom_text_fixture.hpp"

#include "../src/board.hpp"
#include "../src/move.hpp"
#include "../src/zobrist.hpp"

CUSTOM_TEST_CASE("Test Castling Rights") {
	Board b = Board();

	CHECK(b.boardState.castlingRights.rights == 0b1111);
	SUBCASE("Test setWhiteKS") {
		unsigned int bitForWhiteKS = 0b1000;
		b.boardState.castlingRights.setWhiteKS(true);
		CHECK((bool)(b.boardState.castlingRights.rights & bitForWhiteKS));
		b.boardState.castlingRights.setWhiteKS(false);
		CHECK((bool)(b.boardState.castlingRights.rights & ~bitForWhiteKS));
		b.boardState.castlingRights.setWhiteKS(true);
		CHECK((bool)(b.boardState.castlingRights.rights & bitForWhiteKS));
	}
	SUBCASE("Test setWhiteQS") {
		unsigned int bitForWhiteQS = 0b0100;
		b.boardState.castlingRights.setWhiteQS(true);
		CHECK((bool)(b.boardState.castlingRights.rights & bitForWhiteQS));
		b.boardState.castlingRights.setWhiteQS(false);
		CHECK((bool)(b.boardState.castlingRights.rights & ~bitForWhiteQS));
		b.boardState.castlingRights.setWhiteQS(true);
		CHECK((bool)(b.boardState.castlingRights.rights & bitForWhiteQS));
	}
	SUBCASE("Test setBlackKS") {
		unsigned int bitForBlackKS = 0b0010;
		b.boardState.castlingRights.setBlackKS(true);
		CHECK((bool)(b.boardState.castlingRights.rights & bitForBlackKS));
		b.boardState.castlingRights.setBlackKS(false);
		CHECK((bool)(b.boardState.castlingRights.rights & ~bitForBlackKS));
		b.boardState.castlingRights.setBlackKS(true);
		CHECK((bool)(b.boardState.castlingRights.rights & bitForBlackKS));
	}
	SUBCASE("Test setBlackQS") {
		unsigned int bitForBlackQS = 0b0001;
		b.boardState.castlingRights.setBlackQS(true);
		CHECK((bool)(b.boardState.castlingRights.rights & bitForBlackQS));
		b.boardState.castlingRights.setBlackQS(false);
		CHECK((bool)(b.boardState.castlingRights.rights & ~bitForBlackQS));
		b.boardState.castlingRights.setBlackQS(true);
		CHECK((bool)(b.boardState.castlingRights.rights & bitForBlackQS));
	}
}
CUSTOM_TEST_CASE("Test setToFen") {
	Board b = Board();
	SUBCASE("Test fen r1bk1bnr/p1p2ppp/1pnp4/1B2p3/4P2q/P1N2N1P/1PPP1PP1/R1BQK2R w KQ - 0 7") {
		b.setToFen("r1bk1bnr/p1p2ppp/1pnp4/1B2p3/4P2q/P1N2N1P/1PPP1PP1/R1BQK2R w KQ - 0 7");
		SUBCASE("Test sideToMove") {
			CHECK(b.boardState.sideToMove == WHITE);
		}
		SUBCASE("Test enPassantSquare") {
			CHECK(b.boardState.enPassantSquare == 0);
		}
		SUBCASE("Test hmClock") {
			CHECK(b.boardState.hmClock == 0);
		}
		SUBCASE("Test fmClock") {
			CHECK(b.boardState.fmClock == 7);
		}
		SUBCASE("Test whitePieces") {
			CHECK(b.whitePieces() == 8869211805UL);
		}
		SUBCASE("Test blackPieces") {
			CHECK(b.blackPieces() == 17142123020685410304UL);
		}
	}
	SUBCASE("Test fen r3k2r/pp2bppp/2n1p3/q2pPn2/N2P2P1/4BB1P/PP3P2/R2Q1RK1 b kq g3 0 14") {
		b.setToFen("r3k2r/pp2bppp/2n1p3/q2pPn2/N2P2P1/4BB1P/PP3P2/R2Q1RK1 b kq g3 0 14");
		SUBCASE("Test sideToMove") {
			CHECK(b.boardState.sideToMove == BLACK);
		}
		SUBCASE("Test enPassantSquare") {
			CHECK(b.boardState.enPassantSquare == 1UL << g3);
		}
		SUBCASE("Test hmClock") {
			CHECK(b.boardState.hmClock == 0);
		}
		SUBCASE("Test fmClock") {
			CHECK(b.boardState.fmClock == 14);
		}
		SUBCASE("Test whitePieces") {
			CHECK(b.whitePieces() == 69955756905UL);
		}
		SUBCASE("Test blackPieces") {
			CHECK(b.blackPieces() == 10516771721166454784UL);
		}
	}
	SUBCASE("Test fen 2rqrnk1/pp2bpp1/2p1bn1p/3p4/3P3B/2NBPP2/PPQ1N1PP/3R1RK1 w - - 3 14") {
		b.setToFen("2rqrnk1/pp2bpp1/2p1bn1p/3p4/3P3B/2NBPP2/PPQ1N1PP/3R1RK1 w - - 3 14");
		SUBCASE("Test sideToMove") {
			CHECK(b.boardState.sideToMove == WHITE);
		}
		SUBCASE("Test enPassantSquare") {
			CHECK(b.boardState.enPassantSquare == 0);
		}
		SUBCASE("Test hmClock") {
			CHECK(b.boardState.hmClock == 3);
		}
		SUBCASE("Test fmClock") {
			CHECK(b.boardState.fmClock == 14);
		}
		SUBCASE("Test whitePieces") {
			CHECK(b.whitePieces() == 2285688680UL);
		}
		SUBCASE("Test blackPieces") {
			CHECK(b.blackPieces() == 8967709229477527552UL);
		}
	}
}

CUSTOM_TEST_CASE("Test Illegal Check") {
	Board b = Board();
	SUBCASE("Test fen r1bk1bnr/p1p2ppp/1pnp4/1B2p3/4P2q/P1N2N1P/1PPP1PP1/R1BQK2R w KQ - 0 7") {
		b.setToFen("r1bk1bnr/p1p2ppp/1pnp4/1B2p3/4P2q/P1N2N1P/1PPP1PP1/R1BQK2R w KQ - 0 7");
		CHECK(!b.inIllegalCheck());
	}
	SUBCASE("Test fen rnb1kbnr/ppp1pppp/4q3/8/8/2N2N2/PPPP1PPP/R1BQKB1R b KQkq - 3 4") {
		b.setToFen("rnb1kbnr/ppp1pppp/4q3/8/8/2N2N2/PPPP1PPP/R1BQKB1R b KQkq - 3 4");
		CHECK(b.inIllegalCheck());
	}
	SUBCASE("Test fen 8/8/8/4k3/3K4/8/8/8 w - - 0 1") {
		b.setToFen("8/8/8/4k3/3K4/8/8/8 w - - 0 1");
		CHECK(b.inIllegalCheck());
	}
}

CUSTOM_TEST_CASE("Test undoMove") {
	SUBCASE("Undo Regular Move") {
		Board b;
		b.setToFen("r1bk1bnr/p1p2ppp/1pnp4/1B2p3/4P2q/P1N2N1P/1PPP1PP1/R1BQK2R w KQ - 0 7");
		Board::BoardState beforeState = b.boardState;
		Move m = Move(b, g2, g3, PAWN);
		b.execute(m);
		b.undoMove();
		CHECK(b.boardState.pieces == beforeState.pieces);
		CHECK(b.boardState.sideToMove == beforeState.sideToMove);
		CHECK(b.boardState.castlingRights.rights == beforeState.castlingRights.rights);
	}
	SUBCASE("Undo Double Pawn Push") {
		Board b;
		Move m = Move(b, e2, e4, PAWN);
		b.execute(m);
		CHECK(b.boardState.enPassantSquare == 1UL << e3);
		b.undoMove();
		CHECK(b.boardState.enPassantSquare == 0);
	}
	SUBCASE("Undo Capture") {
		Board b;
		b.setToFen("r1bk1bnr/p1p2ppp/1pnp4/1B2p3/4P2q/P1N2N1P/1PPP1PP1/R1BQK2R w KQ - 0 7");
		Board::BoardState beforeState = b.boardState;
		Move m = Move(b, f3, h4, KNIGHT);
		b.execute(m);
		b.undoMove();
		CHECK(b.boardState.pieces == beforeState.pieces);
	}
	SUBCASE("Undo En Passant") {
		Board b;
		b.setToFen("rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3");
		Board::BoardState beforeState = b.boardState;
		Move m = Move(b, e5, f6, PAWN);
		b.execute(m);
		b.undoMove();
		CHECK(b.boardState.pieces == beforeState.pieces);
	}
	SUBCASE("Undo Promotion") {
		Board b;
		b.setToFen("8/1P6/4k3/1K6/8/8/8/8 w - - 0 1");
		Board::BoardState beforeState = b.boardState;
		Move m = Move(b, b7, b8, PAWN, QUEEN);
		b.execute(m);
		b.undoMove();
		CHECK(b.boardState.pieces == beforeState.pieces);
	}
	SUBCASE("Undo Castling") {
		Board b;
		b.setToFen("r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4");
		Board::BoardState beforeState = b.boardState;
		Move m = Move(b, e1, g1, KING);
		b.execute(m);
		b.undoMove();
		CHECK(b.boardState.pieces == beforeState.pieces);
	}
}

CUSTOM_TEST_CASE("Test is50MoveRule") {
	Board b;
	SUBCASE("Not at 50 move rule") {
		b.setToFen("r1bk1bnr/p1p2ppp/1pnp4/1B2p3/4P2q/P1N2N1P/1PPP1PP1/R1BQK2R w KQ - 50 7");
		CHECK(!b.is50MoveRule());
	}
	SUBCASE("At 50 move rule") {
		b.setToFen("r1bk1bnr/p1p2ppp/1pnp4/1B2p3/4P2q/P1N2N1P/1PPP1PP1/R1BQK2R w KQ - 100 7");
		CHECK(b.is50MoveRule());
	}
}

CUSTOM_TEST_CASE("Test isInsufficientMaterial") {
	Board b;
	SUBCASE("Regular Position") {
		b.setToFen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
		CHECK(!b.isInsufficientMaterial());
	}
	SUBCASE("King vs King") {
		b.setToFen("8/8/8/4k3/8/3K4/8/8 w - - 0 1");
		CHECK(b.isInsufficientMaterial());
	}
	SUBCASE("King and Bishop vs King") {
		b.setToFen("8/8/4k3/8/3K1B2/8/8/8 w - - 0 1");
		CHECK(b.isInsufficientMaterial());
	}
	SUBCASE("King and Knight vs King") {
		b.setToFen("8/8/4k3/8/3K2N1/8/8/8 w - - 0 1");
		CHECK(b.isInsufficientMaterial());
	}
	SUBCASE("King and Bishop vs King and Bishop (Same color)") {
		b.setToFen("8/3k1b2/8/8/8/3K4/4B3/8 w - - 0 1");
		CHECK(b.isInsufficientMaterial());
	}
	SUBCASE("King and Bishop vs King and Bishop (Opposite color)") {
		b.setToFen("5b2/3k4/8/8/8/3K4/4B3/8 w - - 0 1");
		CHECK(!b.isInsufficientMaterial());
	}
	SUBCASE("King and two Knights vs King") {
		b.setToFen("8/8/8/4k3/3K2N1/8/4N3/8 w - - 0 1");
		CHECK(!b.isInsufficientMaterial());
	}
	SUBCASE("King and Knight vs King and Knight") {
		b.setToFen("5n2/3k4/8/8/8/3K4/4N3/8 w - - 0 1");
		CHECK(!b.isInsufficientMaterial());
	}
	SUBCASE("King and Knight vs King and Bishop") {
		b.setToFen("5b2/3k4/8/8/8/3K4/4N3/8 w - - 0 1");
		CHECK(!b.isInsufficientMaterial());
	}
	SUBCASE("King and Pawn vs King") {
		b.setToFen("8/8/8/4k3/8/3K4/4P3/8 w - - 0 1");
		CHECK(!b.isInsufficientMaterial());
	}
	SUBCASE("King and 2 Bishops vs King (Opposite color)") {
		b.setToFen("8/3k4/8/8/8/3K4/4BB2/8 w - - 0 1");
		CHECK(!b.isInsufficientMaterial());
	}
	SUBCASE("King and 2 Bishops vs King (Same color)") {
		b.setToFen("8/3k4/8/8/8/3K4/4B3/5B2 w - - 0 1");
		CHECK(b.isInsufficientMaterial());
	}
	SUBCASE("King and 3 Bishops vs King (Same color)") {
		b.setToFen("8/3k4/8/8/8/3K4/4B1B1/5B2 w - - 0 1");
		CHECK(b.isInsufficientMaterial());
	}
}

CUSTOM_TEST_CASE("Test isGameOver") {
	Board b;
	SUBCASE("Not game over") {
		b.setToFen("r1bk1bnr/p1p2ppp/1pnp4/1B2p3/4P2q/P1N2N1P/1PPP1PP1/R1BQK2R w KQ - 0 7");
		CHECK(!b.isGameOver());
	}
	SUBCASE("50 move rule") {
		b.setToFen("r1bk1bnr/p1p2ppp/1pnp4/1B2p3/4P2q/P1N2N1P/1PPP1PP1/R1BQK2R w KQ - 100 7");
		CHECK(b.isGameOver());
	}
	SUBCASE("Insufficient material") {
		b.setToFen("8/8/8/4k3/3K4/8/8/8 w - - 0 1");
		CHECK(b.isGameOver());
	}
}

CUSTOM_TEST_CASE("Test Zobrist Hash") {
	Board b;
	SUBCASE("Initial hash matches") {
		CHECK(b.boardState.hash == Zobrist::initialHash());
	}
	SUBCASE("Hash changes after move") {
		ZobristHash beforeHash = b.boardState.hash;
		Move m = Move(b, e2, e3, PAWN);
		b.execute(m);
		CHECK(b.boardState.hash != beforeHash);
		b.undoMove();
		CHECK(b.boardState.hash == beforeHash);
	}
	SUBCASE("Same position has same hash") {
		Board b1;
		Board b2;
		b1.setToFen("r1bk1bnr/p1p2ppp/1pnp4/1B2p3/4P2q/P1N2N1P/1PPP1PP1/R1BQK2R w KQ - 0 7");
		b2.setToFen("r1bk1bnr/p1p2ppp/1pnp4/1B2p3/4P2q/P1N2N1P/1PPP1PP1/R1BQK2R w KQ - 0 7");
		CHECK(b1.boardState.hash == b2.boardState.hash);
	}
}
