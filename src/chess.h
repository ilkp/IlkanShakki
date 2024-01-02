#pragma once
#include <cstdint>
#include <array>
#include <random>
#include <algorithm>

using GameHash = uint64_t;
static const int BOARD_WIDTH = 8;
static const int BOARD_SIZE = BOARD_WIDTH * BOARD_WIDTH;

enum Side
{
	WHITE,
	BLACK
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
	uint64_t Turn(Side side) const
	{
		return side == Side::WHITE ? _whiteTurn : _blackTurn;
	}
	uint64_t WhiteTurn() const { return _whiteTurn; }
	uint64_t WhiteLongCastle() const { return _whiteLongCastle; }
	uint64_t WhiteShortCastle() const { return _whiteShortCastle; }

	uint64_t BlackTurn() const { return _blackTurn; }
	uint64_t BlackLongCastle() const { return _blackLongCastle; }
	uint64_t BlackShortCastle() const { return _blackShortCastle; }

	uint64_t Piece(Piece piece, int x, int y) const
	{
		return _pieces.at(y * Piece::MAX_VALUE * BOARD_WIDTH + x * Piece::MAX_VALUE + piece);
	}
	uint64_t WhiteEnPassant(int index) const { return _whiteEnPassants.at(index); }
	uint64_t BlackEnPassant(int index) const { return _blackEnPassants.at(index); }
	uint64_t WhiteEnPassantAvailable(int index) const { return _whiteEnPassantsAvailable.at(index); }
	uint64_t BlackEnPassantAvailable(int index) const { return _blackEnPassantsAvailable.at(index); }

	ZobristTable()
	{
		std::random_device rd;
		std::mt19937_64 engine(rd());
		std::uniform_int_distribution<uint64_t> dist;
		std::vector<uint64_t> usedValues;

		_whiteTurn = uniqueRandom64(usedValues, dist, engine);
		_whiteLongCastle = uniqueRandom64(usedValues,  dist, engine);
		_whiteShortCastle = uniqueRandom64(usedValues, dist, engine);

		_blackTurn = uniqueRandom64(usedValues, dist, engine);
		_blackLongCastle = uniqueRandom64(usedValues, dist, engine);
		_blackShortCastle = uniqueRandom64(usedValues, dist, engine);

		for (uint64_t& piece : _pieces)
			piece = uniqueRandom64(usedValues, dist, engine);

		for (uint64_t& enPassant : _whiteEnPassants)
			enPassant = uniqueRandom64(usedValues, dist, engine);

		for (uint64_t& enPassant : _whiteEnPassantsAvailable)
			enPassant = uniqueRandom64(usedValues, dist, engine);

		for (uint64_t& enPassant : _blackEnPassants)
			enPassant = uniqueRandom64(usedValues, dist, engine);

		for (uint64_t& enPassant : _blackEnPassantsAvailable)
			enPassant = uniqueRandom64(usedValues, dist, engine);
	}

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
		const std::mt19937_64& engine) const
	{
		uint64_t value;
		do
		{
			value = dist(engine);
		} while (std::find(usedValues.begin(), usedValues.end(), value) != usedValues.end());
		usedValues.push_back(value);
		return value;
	}
};

GameState initialGameState()
{
	GameState gameState;
	gameState.turn = Side::WHITE;

	gameState.whiteEnPassant = -1;
	gameState.blackEnPassant = -1;
	gameState.whiteLongCastleAvailable = true;
	gameState.whiteShortCastleAvailable = true;
	gameState.blackLongCastleAvailable = true;
	gameState.blackShortCastleAvailable = true;

	for (int i = 0; i < BOARD_WIDTH; ++i)
	{
		gameState.whiteEnPassantsAvailable[i] = true;
		gameState.blackEnPassantsAvailable[i] = true;
	}

	for (uint64_t i = 0; i < BOARD_WIDTH; ++i)
	{
		gameState.pieces[BOARD_WIDTH + i] = Piece::W_PAWN;
		gameState.pieces[6 * BOARD_WIDTH + i] = Piece::B_PAWN;
	}

	gameState.pieces[BOARD_WIDTH + 0] = Piece::W_ROOK;
	gameState.pieces[BOARD_WIDTH + 1] = Piece::W_KNIGHT;
	gameState.pieces[BOARD_WIDTH + 2] = Piece::W_BISHOP;
	gameState.pieces[BOARD_WIDTH + 3] = Piece::W_QUEEN;
	gameState.pieces[BOARD_WIDTH + 4] = Piece::W_KING;
	gameState.pieces[BOARD_WIDTH + 5] = Piece::W_BISHOP;
	gameState.pieces[BOARD_WIDTH + 6] = Piece::W_KNIGHT;
	gameState.pieces[BOARD_WIDTH + 7] = Piece::W_ROOK;

	const int blackPiecesRow = 6 * BOARD_WIDTH;
	gameState.pieces[blackPiecesRow + 0] = Piece::B_ROOK;
	gameState.pieces[blackPiecesRow + 1] = Piece::B_KNIGHT;
	gameState.pieces[blackPiecesRow + 2] = Piece::B_BISHOP;
	gameState.pieces[blackPiecesRow + 3] = Piece::B_QUEEN;
	gameState.pieces[blackPiecesRow + 4] = Piece::B_KING;
	gameState.pieces[blackPiecesRow + 5] = Piece::B_BISHOP;
	gameState.pieces[blackPiecesRow + 6] = Piece::B_KNIGHT;
	gameState.pieces[blackPiecesRow + 7] = Piece::B_ROOK;

	for (int y = 2; y < 6; ++y)
		for (int x = 0; x < BOARD_WIDTH; ++x)
			gameState.pieces[y * BOARD_WIDTH + x] = Piece::NONE;
}

GameHash hashGameState(const GameState& gameState, const ZobristTable& zobristTable)
{
	GameHash gameHash = 0;

	gameHash ^= zobristTable.Turn(gameState.turn);

	if (gameState.whiteEnPassant != -1)
		gameHash ^= zobristTable.WhiteEnPassant(gameState.whiteEnPassant);
	if (gameState.blackEnPassant != -1)
		gameHash ^= zobristTable.WhiteEnPassant(gameState.blackEnPassant);
	if (gameState.whiteLongCastleAvailable)
		gameHash ^= zobristTable.WhiteLongCastle();
	if (gameState.whiteShortCastleAvailable)
		gameHash ^= zobristTable.WhiteShortCastle();
	if (gameState.blackLongCastleAvailable)
		gameHash ^= zobristTable.BlackLongCastle();
	if (gameState.blackShortCastleAvailable)
		gameHash ^= zobristTable.BlackShortCastle();

	for (int i = 0; i < BOARD_WIDTH; ++i)
	{
		if (gameState.whiteEnPassantsAvailable[i])
			gameHash ^= zobristTable.WhiteEnPassantAvailable(i);
		if (gameState.blackEnPassantsAvailable[i])
			gameHash ^= zobristTable.BlackEnPassantAvailable(i);
	}

	for (int y = 0; y < BOARD_WIDTH; ++y)
		for (int x = 0; x < BOARD_WIDTH; ++x)
			gameHash ^= zobristTable.Piece(gameState.pieces.at(y * BOARD_WIDTH + x), x, y);
}

GameHash applyMoveHash(const GameHash gameHash, const MoveHash move)
{
	GameHash nextHash = gameHash;
	for (const uint64_t remove : move.remove)
		nextHash ^= remove;
	for (const uint64_t add : move.add)
		nextHash ^= add;
	return nextHash;
}

double evaluate(const GameState& gameState)
{
	double value = 0;
	for (Piece piece : gameState.pieces)
	{
		switch (piece)
		{
		case W_QUEEN :
			value += 9;
			break;
		case W_BISHOP: case W_KNIGHT:
			value += 3;
			break;
		case W_ROOK:
			value += 5;
			break;
		case W_PAWN:
			value += 1;
			break;
		case B_QUEEN:
			value -= 9;
			break;
		case B_BISHOP: case B_KNIGHT:
			value -= 3;
			break;
		case B_ROOK:
			value -= 5;
			break;
		case B_PAWN:
			value -= 1;
			break;
		default:
			break;
		}
	}
	return value;
}