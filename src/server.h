#pragma once

#include <thread>
#include <vector>

#include "socket.h"

class GameState;
class Move;

class Server
{
public:
	Server(const std::string& port);
	Server(const Server& other) = delete;
	Server(Server&& other) = delete;
	Server& operator=(const Server& other) = delete;
	Server& operator=(Server&& other) = delete;
	~Server();

private:
	enum class State
	{
		STARTING,
		PLAYING,
		GAME_OVER
	};

	const std::string _port;
	std::thread _mainLoopThread;
	SOCKET _listenSocket;
	bool _closing;

	void mainLoop();

	std::vector<char> tcpMsgGameState(const GameState& gameState);
	std::vector<char> tcpMsgRequestMove();
};