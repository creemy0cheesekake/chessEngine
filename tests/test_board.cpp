#include "../include/doctest.h"
#include "custom_text_fixture.hpp"

#include "../src/board.hpp"

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
