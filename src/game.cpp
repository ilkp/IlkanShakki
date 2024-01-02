#include "game.h"

Game::Game(long long clock_ms) :
	GAME_CLOCK(clock_ms),
	_isClosing(false)
{
	_clocks_ms[0] = 0;
	_clocks_ms[1] = 0;
}

void Game::MainLoop()
{
	_clocks_ms[0] = GAME_CLOCK;
	_clocks_ms[1] = GAME_CLOCK;
	_gameState = initialGameState();

	auto lastTimePoint = std::chrono::system_clock::now();
	while(!_isClosing)
	{
		auto now = std::chrono::system_clock::now();
		long long elapsedTime_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTimePoint).count();
		lastTimePoint = now;
		_clocks_ms[_gameState.turn] -= elapsedTime_ms;

		if (_clocks_ms[_gameState.turn] <= 0)
		{
			if (hasWinner(_gameState).hasValue)
			{

			}
		}
	}
}

Game::Winner Game::hasWinner(const GameState& gameState)
{
	return Winner();
}
