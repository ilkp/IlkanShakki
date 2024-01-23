#pragma once

#include "chess.h"
#include "socket.h"

#include <thread>
#include <vector>
#include <array>
#include <functional>
#include <chrono>

class Server
{
public:
	Server(const std::string& port);
	Server(const Server& other) = delete;
	Server(Server&& other) = delete;
	Server& operator=(const Server& other) = delete;
	Server& operator=(Server&& other) = delete;
	~Server();
	std::thread _serverThread;

private:
	enum class ServerState
	{
		STARTING,
		PLAYING,
		GAME_OVER,
		CLOSING
	};

	enum class PlayState
	{
		EVALUATE_GAME,
		SEND_REQUEST_MOVE,
		WAIT_FOR_CLIENT_MOVE,
		MOVE_RECEIVED,
		END_GAME
	};

	struct PlayStateBlackboard
	{
		PlayState playState;
		GameState gameState;
		Move move;
		std::chrono::steady_clock::time_point startTime;
		std::array<int, 2> clocks_ms;
	};

	SOCKET _listenSocket;
	std::vector<SOCKET> _playerSockets;
	ServerState _serverState;
	std::atomic_bool _closeRequested;

	void nextState();
	void serverStateStarting();
	void serverStatePlaying();

	void playStateEvaluateGame(PlayStateBlackboard& blackboard);
	void playStateSendRequestMove(PlayStateBlackboard& blackboard);
	void playStateWaitForClientMove(PlayStateBlackboard& blackboard);
	void playStateMoveReceived(PlayStateBlackboard& blackboard);
	void playStateEndGame(PlayStateBlackboard& blackboard);

	std::vector<char> gameStateTcpMsg(const GameState& gameState);
	std::vector<char> requestMoveTcpMsg();
};