#pragma once
#include <cstdint>
#include <array>
#include <random>
#include <algorithm>

using GameHash = uint64_t;
static const uint64_t BOARD_WIDTH = 8;
static const uint64_t BOARD_SIZE = BOARD_WIDTH * BOARD_WIDTH;

enum Side
{
	WHITE,
	BLACK,
	NONE
};

enum Piece : char
{
	NONE,
	W_KING,
	W_QUEEN,
	W_BISHOP,
	W_KNIGHT,
	W_ROOK,
	W_PAWN,
	B_KING,
	B_QUEEN,
	B_BISHOP,
	B_KNIGHT,
	B_ROOK,
	B_PAWN,
	MAX_VALUE
};

struct GameState
{
	Side turn;
	bool whiteLongCastleAvailable;
	bool whiteShortCastleAvailable;
	bool blackLongCastleAvailable;
	bool blackShortCastleAvailable;
	char whiteEnPassant;
	char blackEnPassant;
	std::array<bool, BOARD_WIDTH> whiteEnPassantsAvailable;
	std::array<bool, BOARD_WIDTH> blackEnPassantsAvailable;
	std::array<Piece, BOARD_SIZE> pieces;
};

struct Move
{
	int xFrom;
	int yFrom;
	int xTo;
	int yTo;
};

struct MoveHash
{
	std::vector<uint64_t> remove;
	std::vector<uint64_t> add;
};

class ZobristTable
{
public:
	ZobristTable();

	uint64_t Turn(Side side) const;
	uint64_t WhiteTurn() const;
	uint64_t WhiteLongCastle() const;
	uint64_t WhiteShortCastle() const;

	uint64_t BlackTurn() const;
	uint64_t BlackLongCastle() const;
	uint64_t BlackShortCastle() const;

	uint64_t PieceHash(Piece piece, int x, int y) const;
	uint64_t WhiteEnPassant(int index) const;
	uint64_t BlackEnPassant(int index) const;
	uint64_t WhiteEnPassantAvailable(int index) const;
	uint64_t BlackEnPassantAvailable(int index) const;

private:
	static const int TOTAL_PIECE_HASHES = BOARD_SIZE * Piece::MAX_VALUE;

	uint64_t _whiteTurn;
	uint64_t _whiteLongCastle;
	uint64_t _whiteShortCastle;

	uint64_t _blackTurn;
	uint64_t _blackLongCastle;
	uint64_t _blackShortCastle;

	std::array<uint64_t, BOARD_WIDTH> _whiteEnPassants;
	std::array<uint64_t, BOARD_WIDTH> _whiteEnPassantsAvailable;
	std::array<uint64_t, BOARD_WIDTH> _blackEnPassants;
	std::array<uint64_t, BOARD_WIDTH> _blackEnPassantsAvailable;

	std::array<uint64_t, TOTAL_PIECE_HASHES> _pieces;

	uint64_t uniqueRandom64(
		std::vector<uint64_t>& usedValues,
		std::uniform_int_distribution<uint64_t>& dist,
		std::mt19937_64& engine) const;
};

GameState defaultStartGameState();

GameHash hashGameState(const GameState& gameState, const ZobristTable& zobristTable);

GameHash applyMoveHash(const GameHash gameHash, const MoveHash move);

double evaluate(const GameState& gameState);

Side evaluateWinner(const GameState& gameState);

bool moveIsLegal(const Move& move, const GameState& gameState);

void applyMove(const Move& move, GameState& gameState);