#include <iostream>
#include "../include/doctest.h"
#include "custom_text_fixture.hpp"

#include "../src/move.hpp"

CUSTOM_TEST_CASE("Test Pawn Moves") {
	SUBCASE("Test Pawn Promotion") {
		Board b = Board();
		b.setToFen("8/1P1k4/8/8/1K6/8/8/8 w - - 1 9");
		Move m = Move(b, b7, b8, PAWN, QUEEN);
		b	   = m.execute();
		CHECK(m.getFlags() == Move::PAWN_MOVE + Move::PROMOTION);
		CHECK(b.hmClock == 0);
		CHECK(b.fmClock == 9);
		CHECK(b.sideToMove == BLACK);
	}
	SUBCASE("Test Double Pawn Push") {
		Move m	= Move(Board(), e2, e4, PAWN);
		Board b = m.execute();
		CHECK(m.getFlags() == Move::DBL_PAWN + Move::PAWN_MOVE);
		CHECK(b.hmClock == 0);
		CHECK(b.fmClock == 1);
		CHECK(b.enPassantSquare == 1UL << e3);
		CHECK(b.sideToMove == BLACK);
	}
	SUBCASE("Test Pawn Capture") {
		Board b = Board();
		b.setToFen("rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2");
		Move m = Move(b, e4, d5, PAWN);
		b	   = m.execute();
		CHECK(m.getFlags() == Move::CAPTURE + Move::PAWN_MOVE);
		CHECK(b.hmClock == 0);
		CHECK(b.fmClock == 2);
		CHECK(b.sideToMove == BLACK);
	}
	SUBCASE("Test En Passant") {
		Board b = Board();
		b.setToFen("rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3");
		Move m = Move(b, e5, f6, PAWN);
		CHECK(b.enPassantSquare == 1UL << f6);
		b = m.execute();
		CHECK(m.getFlags() == Move::CAPTURE + Move::EN_PASSANT + Move::PAWN_MOVE);
		CHECK(b.hmClock == 0);
		CHECK(b.fmClock == 3);
		CHECK(b.sideToMove == BLACK);
	}
}

CUSTOM_TEST_CASE("Test Castling") {
	SUBCASE("White Kingside Castling") {
		Board b = Board();
		b.setToFen("r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4");
		Move m = Move(b, e1, g1, KING);
		b	   = m.execute();
		CHECK(m.getFlags() == Move::KS_CASTLE);
		CHECK(b.hmClock == 5);
		CHECK(b.fmClock == 4);
		CHECK(b.sideToMove == BLACK);
		CHECK((bool)(b.castlingRights.rights & ~0b1100));
	}
	SUBCASE("White Queenside Castling") {
		Board b = Board();
		b.setToFen("rnbq1rk1/ppp1bppp/4pn2/3p2B1/2PP4/2N5/PPQ1PPPP/R3KBNR w KQ - 6 6");
		Move m = Move(b, e1, c1, KING);
		b	   = m.execute();
		CHECK(m.getFlags() == Move::QS_CASTLE);
		CHECK(b.hmClock == 7);
		CHECK(b.fmClock == 6);
		CHECK(b.sideToMove == BLACK);
		CHECK((bool)(~b.castlingRights.rights & ~0b1100));
	}
	SUBCASE("Black Kingside Castling") {
		Board b = Board();
		b.setToFen("rnbqk2r/ppp1bppp/4pn2/3p2B1/2PP4/2N5/PPQ1PPPP/R3KBNR b KQkq - 5 5");
		Move m = Move(b, e8, g8, KING);
		b	   = m.execute();
		CHECK(m.getFlags() == Move::KS_CASTLE);
		CHECK(b.hmClock == 6);
		CHECK(b.fmClock == 6);
		CHECK(b.sideToMove == WHITE);
		CHECK((bool)(b.castlingRights.rights & ~0b0011));
	}
	SUBCASE("Black Queenside Castling") {
		Board b = Board();
		b.setToFen("r3kbnr/pp2pppp/2nq4/2pp1b2/3P4/2P1PN2/PP2BPPP/RNBQ1RK1 b kq - 4 6");
		Move m = Move(b, e8, c8, KING);
		b	   = m.execute();
		CHECK(m.getFlags() == Move::QS_CASTLE);
		CHECK(b.hmClock == 5);
		CHECK(b.fmClock == 7);
		CHECK(b.sideToMove == WHITE);
		CHECK((bool)(~b.castlingRights.rights & ~0b0011));
	}
}

CUSTOM_TEST_CASE("Test Rook Affecting Castling") {
	SUBCASE("White Kingside Castling") {
		Board b = Board();
		b.setToFen("rnbqkb1r/pppppppp/5n2/8/8/5N2/PPPPPPPP/RNBQKB1R w KQkq - 2 2");
		Move m = Move(b, h1, g1, ROOK);
		b	   = m.execute();
		CHECK(m.getFlags() == Move::NORMAL_MOVE);
		CHECK(b.hmClock == 3);
		CHECK(b.fmClock == 2);
		CHECK(b.sideToMove == BLACK);
		CHECK((bool)(~b.castlingRights.rights & ~0b1));
	}
	SUBCASE("White Queenside Castling") {
		Board b = Board();
		b.setToFen("rnbqkbnr/pppp1ppp/8/4p3/P7/8/1PPPPPPP/RNBQKBNR w KQkq - 0 2");
		Move m = Move(b, a1, a2, ROOK);
		b	   = m.execute();
		CHECK(m.getFlags() == Move::NORMAL_MOVE);
		CHECK(b.hmClock == 1);
		CHECK(b.fmClock == 2);
		CHECK(b.sideToMove == BLACK);
		CHECK((bool)(~b.castlingRights.rights & ~0b01));
	}
	SUBCASE("Black Kingside Castling") {
		Board b = Board();
		b.setToFen("rnbqkbnr/ppppppp1/8/7p/4P3/2N5/PPPP1PPP/R1BQKBNR b KQkq - 1 2");
		Move m = Move(b, h8, h7, ROOK);
		b	   = m.execute();
		CHECK(m.getFlags() == Move::NORMAL_MOVE);
		CHECK(b.hmClock == 2);
		CHECK(b.fmClock == 3);
		CHECK(b.sideToMove == WHITE);
		CHECK((bool)(~b.castlingRights.rights & ~0b001));
	}
	SUBCASE("Black Queenside Castling") {
		Board b = Board();
		b.setToFen("rnbqkbnr/1ppppppp/8/p7/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2");
		Move m = Move(b, a8, a7, ROOK);
		b	   = m.execute();
		CHECK(m.getFlags() == Move::NORMAL_MOVE);
		CHECK(b.hmClock == 2);
		CHECK(b.fmClock == 3);
		CHECK(b.sideToMove == WHITE);
		CHECK((bool)(~b.castlingRights.rights & ~0b0001));
	}
}

CUSTOM_TEST_CASE("Test King Affecting Castling") {
	SUBCASE("White Castling") {
		Board b = Board();
		b.setToFen("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2");
		Move m = Move(b, e1, e2, KING);
		b	   = m.execute();
		CHECK(m.getFlags() == Move::NORMAL_MOVE);
		CHECK(b.hmClock == 1);
		CHECK(b.fmClock == 2);
		CHECK(b.sideToMove == BLACK);
		CHECK((bool)(~b.castlingRights.rights & ~0b11));
	}
	SUBCASE("Black Castling") {
		Board b = Board();
		b.setToFen("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPPKPPP/RNBQ1BNR b kq - 1 2");
		Move m = Move(b, e8, e7, KING);
		b	   = m.execute();
		CHECK(m.getFlags() == Move::NORMAL_MOVE);
		CHECK(b.hmClock == 2);
		CHECK(b.fmClock == 3);
		CHECK(b.sideToMove == WHITE);
		CHECK((bool)(~b.castlingRights.rights & ~0b0011));
	}
}

CUSTOM_TEST_CASE("Test Long Algebraic Notation") {
	SUBCASE("Regular Moves") {
		Board b;
		b.setToFen("r1bk1bnr/p1p2ppp/1pnp4/1B2p3/4P2q/P1N2N1P/1PPP1PP1/R1BQK2R w KQ - 0 7");
		CHECK(Move(b, g2, g3, PAWN).notation() == "g2-g3");
		CHECK(Move(b, h1, f1, ROOK).notation() == "Rh1-f1");
		CHECK(Move(b, c3, d5, KNIGHT).notation() == "Nc3-d5");
	}
	SUBCASE("Captures") {
		Board b;
		b.setToFen("r1bk1bnr/p1p2ppp/1pnp4/1B2p3/4P2q/P1N2N1P/1PPP1PP1/R1BQK2R w KQ - 0 7");
		CHECK(Move(b, f3, h4, KNIGHT).notation() == "Nf3xh4");
		CHECK(Move(b, b5, c6, BISHOP).notation() == "Bb5xc6");
		CHECK(Move(b, f3, e5, KNIGHT).notation() == "Nf3xe5");
	}
	SUBCASE("Kingside Castling") {
		Board b;
		b.setToFen("r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4");
		CHECK(Move(b, e1, g1, KING).notation() == "0-0");
		b.setToFen("r1bqk2r/pppp1ppp/2nb1n2/4p3/2B1P3/2P2N2/PP1P1PPP/RNBQ1RK1 b kq - 0 5");
		CHECK(Move(b, e8, g8, KING).notation() == "0-0");
	}
	SUBCASE("Queenside Castling") {
		Board b;
		b.setToFen("r1bq1rk1/ppppbppp/2n2n2/4p1B1/4P3/2NP4/PPPQ1PPP/R3KBNR w KQ - 5 6");
		CHECK(Move(b, e1, c1, KING).notation() == "0-0-0");
		b.setToFen("r3kbnr/pp2pppp/1qn5/3pP3/3P2b1/5N2/PP2BPPP/RNBQ1RK1 b kq - 6 8");
		CHECK(Move(b, e8, c8, KING).notation() == "0-0-0");
	}
	SUBCASE("Promotion") {
		Board b;
		b.setToFen("8/1P6/4k3/1K6/8/8/8/8 w - - 0 1");
		CHECK(Move(b, b7, b8, PAWN, QUEEN).notation() == "b7-b8=Q");
		CHECK(Move(b, b7, b8, PAWN, ROOK).notation() == "b7-b8=R");
		b.setToFen("8/8/4k3/1K6/8/8/1p6/8 b - - 0 1");
		CHECK(Move(b, b2, b1, PAWN, BISHOP).notation() == "b2-b1=B");
		CHECK(Move(b, b2, b1, PAWN, KNIGHT).notation() == "b2-b1=N");
	}
}

CUSTOM_TEST_CASE("Test UCI Notation") {
	SUBCASE("Regular Moves") {
		Board b;
		b.setToFen("r1bk1bnr/p1p2ppp/1pnp4/1B2p3/4P2q/P1N2N1P/1PPP1PP1/R1BQK2R w KQ - 0 7");
		CHECK(Move(b, g2, g3, PAWN).UCInotation() == "g2g3");
		CHECK(Move(b, h1, f1, ROOK).UCInotation() == "h1f1");
		CHECK(Move(b, c3, d5, KNIGHT).UCInotation() == "c3d5");
	}
	SUBCASE("Captures") {
		Board b;
		b.setToFen("r1bk1bnr/p1p2ppp/1pnp4/1B2p3/4P2q/P1N2N1P/1PPP1PP1/R1BQK2R w KQ - 0 7");
		CHECK(Move(b, f3, h4, KNIGHT).UCInotation() == "f3h4");
		CHECK(Move(b, b5, c6, BISHOP).UCInotation() == "b5c6");
		CHECK(Move(b, f3, e5, KNIGHT).UCInotation() == "f3e5");
	}
	SUBCASE("Kingside Castling") {
		Board b;
		b.setToFen("r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4");
		CHECK(Move(b, e1, g1, KING).UCInotation() == "e1g1");
		b.setToFen("r1bqk2r/pppp1ppp/2nb1n2/4p3/2B1P3/2P2N2/PP1P1PPP/RNBQ1RK1 b kq - 0 5");
		CHECK(Move(b, e8, g8, KING).UCInotation() == "e8g8");
	}
	SUBCASE("Queenside Castling") {
		Board b;
		b.setToFen("r1bq1rk1/ppppbppp/2n2n2/4p1B1/4P3/2NP4/PPPQ1PPP/R3KBNR w KQ - 5 6");
		CHECK(Move(b, e1, c1, KING).UCInotation() == "e1c1");
		b.setToFen("r3kbnr/pp2pppp/1qn5/3pP3/3P2b1/5N2/PP2BPPP/RNBQ1RK1 b kq - 6 8");
		CHECK(Move(b, e8, c8, KING).UCInotation() == "e8c8");
	}
	SUBCASE("Promotion") {
		Board b;
		b.setToFen("8/1P6/4k3/1K6/8/8/8/8 w - - 0 1");
		CHECK(Move(b, b7, b8, PAWN, QUEEN).UCInotation() == "b7b8q");
		CHECK(Move(b, b7, b8, PAWN, ROOK).UCInotation() == "b7b8r");
		b.setToFen("8/8/4k3/1K6/8/8/1p6/8 b - - 0 1");
		CHECK(Move(b, b2, b1, PAWN, BISHOP).UCInotation() == "b2b1b");
		CHECK(Move(b, b2, b1, PAWN, KNIGHT).UCInotation() == "b2b1n");
	}
}

CUSTOM_TEST_CASE("Test execute") {
	SUBCASE("Regular Moves") {
		SUBCASE("Case 1") {
			Board testBoard;
			testBoard.setToFen("r1bk1bnr/p1p2ppp/1pnp4/1B2p3/4P2q/P1N2N1P/1PPP1PP1/R1BQK2R w KQ - 0 7");
			Board refBoard;
			refBoard.setToFen("r1bk1bnr/p1p2ppp/1pnp4/1B2p3/4P2q/P1N2NPP/1PPP1P2/R1BQK2R b KQ - 0 7");

			testBoard = Move(testBoard, g2, g3, PAWN).execute();

			std::array<std::array<Bitboard, 6>, 2> testArr = testBoard.pieces;
			CHECK(testArr == refBoard.pieces);
		}
		SUBCASE("Case 2") {
			Board testBoard;
			testBoard.setToFen("r1bk1bnr/p1p2ppp/1pnp4/1B2p3/4P2q/P1N2N1P/1PPP1PP1/R1BQK2R w KQ - 0 7");
			Board refBoard;
			refBoard.setToFen("r1bk1bnr/p1p2ppp/1pnp4/1B2p3/4P2q/P1N2N1P/1PPP1PP1/R1BQKR2 b Q - 1 7");
			testBoard									   = Move(testBoard, h1, f1, ROOK).execute();
			std::array<std::array<Bitboard, 6>, 2> testArr = testBoard.pieces;
			CHECK(testArr == refBoard.pieces);
		}
		SUBCASE("Case 3") {
			Board testBoard;
			testBoard.setToFen("r1bk1bnr/p1p2ppp/1pnp4/1B2p3/4P2q/P1N2N1P/1PPP1PP1/R1BQK2R w KQ - 0 7");
			Board refBoard;
			refBoard.setToFen("r1bk1bnr/p1p2ppp/1pnp4/1B1Np3/4P2q/P4N1P/1PPP1PP1/R1BQK2R b KQ - 1 7");
			testBoard									   = Move(testBoard, c3, d5, KNIGHT).execute();
			std::array<std::array<Bitboard, 6>, 2> testArr = testBoard.pieces;
			CHECK(testArr == refBoard.pieces);
		}
	}
	SUBCASE("Captures") {
		SUBCASE("Case 1") {
			Board testBoard;
			testBoard.setToFen("r1bk1bnr/p1p2ppp/1pnp4/1B2p3/4P2q/P1N2N1P/1PPP1PP1/R1BQK2R w KQ - 0 7");
			Board refBoard;
			refBoard.setToFen("r1bk1bnr/p1p2ppp/1pnp4/1B2p3/4P2N/P1N4P/1PPP1PP1/R1BQK2R b KQ - 0 7");
			testBoard									   = Move(testBoard, f3, h4, KNIGHT).execute();
			std::array<std::array<Bitboard, 6>, 2> testArr = testBoard.pieces;
			CHECK(testArr == refBoard.pieces);
		}
		SUBCASE("Case 2") {
			Board testBoard;
			testBoard.setToFen("r1bk1bnr/p1p2ppp/1pnp4/1B2p3/4P2q/P1N2N1P/1PPP1PP1/R1BQK2R w KQ - 0 7");
			Board refBoard;
			refBoard.setToFen("r1bk1bnr/p1p2ppp/1pBp4/4p3/4P2q/P1N2N1P/1PPP1PP1/R1BQK2R b KQ - 0 7");
			testBoard									   = Move(testBoard, b5, c6, BISHOP).execute();
			std::array<std::array<Bitboard, 6>, 2> testArr = testBoard.pieces;
			CHECK(testArr == refBoard.pieces);
		}
		SUBCASE("Case 3") {
			Board testBoard;
			testBoard.setToFen("r1bk1bnr/p1p2ppp/1pnp4/1B2p3/4P2q/P1N2N1P/1PPP1PP1/R1BQK2R w KQ - 0 7");
			Board refBoard;
			refBoard.setToFen("r1bk1bnr/p1p2ppp/1pnp4/1B2N3/4P2q/P1N4P/1PPP1PP1/R1BQK2R b KQ - 0 7");
			testBoard									   = Move(testBoard, f3, e5, KNIGHT).execute();
			std::array<std::array<Bitboard, 6>, 2> testArr = testBoard.pieces;
			CHECK(testArr == refBoard.pieces);
		}
	}
	SUBCASE("Kingside Castling") {
		SUBCASE("Case 1") {
			Board testBoard;
			testBoard.setToFen("r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4");
			Board refBoard;
			refBoard.setToFen("r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQ1RK1 b kq - 5 4");
			testBoard									   = Move(testBoard, e1, g1, KING).execute();
			std::array<std::array<Bitboard, 6>, 2> testArr = testBoard.pieces;
			CHECK(testArr == refBoard.pieces);
		}
		SUBCASE("Case 2") {
			Board testBoard;
			testBoard.setToFen("r1bqk2r/pppp1ppp/2nb1n2/4p3/2B1P3/2P2N2/PP1P1PPP/RNBQ1RK1 b kq - 0 5");
			Board refBoard;
			refBoard.setToFen("r1bq1rk1/pppp1ppp/2nb1n2/4p3/2B1P3/2P2N2/PP1P1PPP/RNBQ1RK1 w - - 1 6");
			testBoard									   = Move(testBoard, e8, g8, KING).execute();
			std::array<std::array<Bitboard, 6>, 2> testArr = testBoard.pieces;
			CHECK(testArr == refBoard.pieces);
		}
	}
	SUBCASE("Queenside Castling") {
		SUBCASE("Case 1") {
			Board testBoard;
			testBoard.setToFen("r1bq1rk1/ppppbppp/2n2n2/4p1B1/4P3/2NP4/PPPQ1PPP/R3KBNR w KQ - 5 6");
			Board refBoard;
			refBoard.setToFen("r1bq1rk1/ppppbppp/2n2n2/4p1B1/4P3/2NP4/PPPQ1PPP/2KR1BNR b - - 6 6");
			testBoard									   = Move(testBoard, e1, c1, KING).execute();
			std::array<std::array<Bitboard, 6>, 2> testArr = testBoard.pieces;
			CHECK(testArr == refBoard.pieces);
		}
		SUBCASE("Case 2") {
			Board testBoard;
			testBoard.setToFen("r3kbnr/pp2pppp/1qn5/3pP3/3P2b1/5N2/PP2BPPP/RNBQ1RK1 b kq - 6 8");
			Board refBoard;
			refBoard.setToFen("2kr1bnr/pp2pppp/1qn5/3pP3/3P2b1/5N2/PP2BPPP/RNBQ1RK1 w - - 7 9");
			testBoard									   = Move(testBoard, e8, c8, KING).execute();
			std::array<std::array<Bitboard, 6>, 2> testArr = testBoard.pieces;
			CHECK(testArr == refBoard.pieces);
		}
	}
	SUBCASE("Promotion") {
		SUBCASE("Case 1") {
			Board testBoard;
			testBoard.setToFen("8/1P6/4k3/1K6/8/8/8/8 w - - 0 1");
			Board refBoard;
			refBoard.setToFen("1Q6/8/4k3/1K6/8/8/8/8 b - - 0 1");
			testBoard									   = Move(testBoard, b7, b8, PAWN, QUEEN).execute();
			std::array<std::array<Bitboard, 6>, 2> testArr = testBoard.pieces;
			CHECK(testArr == refBoard.pieces);
		}
		SUBCASE("Case 2") {
			Board testBoard;
			testBoard.setToFen("8/1P6/4k3/1K6/8/8/8/8 w - - 0 1");
			Board refBoard;
			refBoard.setToFen("1R6/8/4k3/1K6/8/8/8/8 b - - 0 1");
			testBoard									   = Move(testBoard, b7, b8, PAWN, ROOK).execute();
			std::array<std::array<Bitboard, 6>, 2> testArr = testBoard.pieces;
			CHECK(testArr == refBoard.pieces);
		}
		SUBCASE("Case 3") {
			Board testBoard;
			testBoard.setToFen("8/8/4k3/1K6/8/8/1p6/8 b - - 0 1");
			Board refBoard;
			refBoard.setToFen("8/8/4k3/1K6/8/8/8/1b6 w - - 0 2");
			testBoard									   = Move(testBoard, b2, b1, PAWN, BISHOP).execute();
			std::array<std::array<Bitboard, 6>, 2> testArr = testBoard.pieces;
			CHECK(testArr == refBoard.pieces);
		}
		SUBCASE("Case 4") {
			Board testBoard;
			testBoard.setToFen("8/8/4k3/1K6/8/8/1p6/8 b - - 0 1");
			Board refBoard;
			refBoard.setToFen("8/8/4k3/1K6/8/8/8/1n6 w - - 0 2");
			testBoard									   = Move(testBoard, b2, b1, PAWN, KNIGHT).execute();
			std::array<std::array<Bitboard, 6>, 2> testArr = testBoard.pieces;
			CHECK(testArr == refBoard.pieces);
		}
	}
}
