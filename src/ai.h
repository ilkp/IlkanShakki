#pragma once
#include <map>
#include <future>

#include "chess.h"

class Ai
{
public:
	std::future<Move> requestMove();

private:
	std::map<uint64_t, double> _evaluatedGameStates;
};