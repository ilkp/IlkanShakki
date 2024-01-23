
#include <exception>
#include <future>
#include <array>
#include <iostream>

#include "server.h"
#include "chess.h"
#include "tcp_messages.h"

Server::Server(const std::string& port) :
	_listenSocket(INVALID_SOCKET),
	_closeRequested(false),
	_serverState(ServerState::STARTING)
{
	_listenSocket = listenSocket(port);
	if (listen(_listenSocket, 10) < 0)
		throw std::exception("Failed to create listening socket");
	_serverThread = std::thread(&Server::nextState, this);
}

Server::~Server()
{
	CLOSESOCKET(_listenSocket);
}

void Server::nextState()
{
	bool closing = false;
	while (!closing)
	{
		switch (_serverState)
		{
			case ServerState::STARTING:
			{
				serverStateStarting();
				break;
			}
			case ServerState::PLAYING:
			{
				serverStatePlaying();
				break;
			}
			case ServerState::CLOSING:
			{
				CLOSESOCKET(_listenSocket);
				for (SOCKET& playerSocket : _playerSockets)
					CLOSESOCKET(playerSocket);
				closing = true;
			}
		}
	}
}

void Server::serverStateStarting()
{
	std::future<SOCKET> pendingSocket;
	for (int i = 0; i < 2; ++i)
	{
		pendingSocket = std::async(accept, _listenSocket, nullptr, nullptr);
		while (_serverState == ServerState::STARTING && pendingSocket.wait_for(std::chrono::seconds(1)) != std::future_status::ready)
		{
			if (_closeRequested)
			{
				shutdown(_listenSocket, SD_RECEIVE);
				_serverState = ServerState::CLOSING;
			}
		}

		if (pendingSocket.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
			_playerSockets.push_back(pendingSocket.get());

		if (_serverState != ServerState::STARTING)
			break;
	}

	if (_serverState == ServerState::STARTING)
		_serverState = ServerState::PLAYING;
}

void Server::serverStatePlaying()
{
	PlayStateBlackboard blackboard;
	blackboard.gameState = defaultStartGameState();
	blackboard.playState = PlayState::EVALUATE_GAME;

	while (_serverState == ServerState::PLAYING)
	{
		switch (blackboard.playState)
		{
			case PlayState::EVALUATE_GAME:
				playStateEvaluateGame(blackboard);
				break;
			case PlayState::SEND_REQUEST_MOVE:
				playStateSendRequestMove(blackboard);
				break;
			case PlayState::WAIT_FOR_CLIENT_MOVE:
				playStateWaitForClientMove(blackboard);
				break;
			case PlayState::MOVE_RECEIVED:
				playStateMoveReceived(blackboard);
				break;
			case PlayState::END_GAME:
				playStateEndGame(blackboard);
				break;
		}
	}
}

void Server::playStateEvaluateGame(PlayStateBlackboard& blackboard)
{
	Side winner = evaluateWinner(blackboard.gameState);
	if (winner != Side::NONE)
	{
		blackboard.playState = PlayState::END_GAME;
	}
	else
	{
		std::vector<char> gameStateMsg = gameStateTcpMsg(blackboard.gameState);
		for (const SOCKET& playerSocket : _playerSockets)
			send(playerSocket, gameStateMsg.data(), gameStateMsg.size(), 0);
		blackboard.playState = PlayState::SEND_REQUEST_MOVE;
	}
}

void Server::playStateSendRequestMove(PlayStateBlackboard& blackboard)
{
	std::vector<char> requestMoveMsg = requestMoveTcpMsg();
	send(_playerSockets[blackboard.gameState.turn], requestMoveMsg.data(), requestMoveMsg.size(), 0);
	blackboard.playState = PlayState::WAIT_FOR_CLIENT_MOVE;
}

void Server::playStateWaitForClientMove(PlayStateBlackboard& blackboard)
{
	std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
	while (blackboard.playState == PlayState::WAIT_FOR_CLIENT_MOVE)
	{
		if (_closeRequested)
		{
			_serverState = ServerState::CLOSING;
			break;
		}

		std::chrono::steady_clock::time_point timeNow = std::chrono::steady_clock::now();
		blackboard.clocks_ms[blackboard.gameState.turn] -= std::chrono::duration_cast<std::chrono::milliseconds>(startTime - timeNow).count();
		startTime = timeNow;
		if (blackboard.clocks_ms[blackboard.gameState.turn] <= 0)
		{
			blackboard.playState = PlayState::EVALUATE_GAME;
			break;
		}
	}
}

void Server::playStateMoveReceived(PlayStateBlackboard& blackboard)
{
	if (moveIsLegal(blackboard.move, blackboard.gameState))
	{
		applyMove(blackboard.move, blackboard.gameState);
		blackboard.playState = PlayState::EVALUATE_GAME;
	}
	else
	{
		blackboard.playState = PlayState::SEND_REQUEST_MOVE;
	}
}

void Server::playStateEndGame(PlayStateBlackboard& blackboard)
{
}

std::vector<char> Server::gameStateTcpMsg(const GameState& gameState)
{
	std::vector<char> msg;
	msg.push_back((char)TcpMsgType::GAME_STATE);

	char* gameStatePtr = (char*)&gameState;
	for (size_t i = 0; i < sizeof(gameState); ++i)
		msg.push_back(*(gameStatePtr + i));

	msg[0] = msg.size();

	return msg;
}

std::vector<char> Server::requestMoveTcpMsg()
{
	std::vector<char> msg;
	msg.push_back((char)TcpMsgType::REQUEST_MOVE);
	return msg;
}
