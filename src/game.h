#pragma once
#include <map>
#include <chrono>
#include <future>

#include "chess.h"

class Game
{
public:
	Game(long long clock_ms);
	void MainLoop();

private:
	struct Winner
	{
		bool hasValue;
		Side side;
	};

	const long long GAME_CLOCK;

	GameState _gameState;
	long long _clocks_ms[2];
	bool _isClosing;
	std::future<Move> _requestedMove;

	Winner hasWinner(const GameState& gameState);
};