
#include <exception>
#include <future>
#include <array>
#include <iostream>

#include "server.h"
#include "chess.h"
#include "tcp_messages.h"

Server::Server(const std::string& port) :
	_port(port),
	_listenSocket(INVALID_SOCKET),
	_closing(false)
{
	_listenSocket = listenSocket(port);
	if (listen(_listenSocket, 10) < 0)
		throw std::exception("Failed to create listening socket");
	_mainLoopThread = std::thread(&Server::mainLoop, this);
}

Server::~Server()
{
	CLOSESOCKET(_listenSocket);
}

void Server::mainLoop()
{
	std::array<SOCKET, 2> playerSockets;

	playerSockets[0] = accept(_listenSocket, nullptr, nullptr);
	if (playerSockets[0] == INVALID_SOCKET)
		throw std::exception("Failed to accept client");

	//playerSockets[1] = accept(_listenSocket, nullptr, nullptr);
	//if (playerSockets[1] == INVALID_SOCKET)
	//	throw std::exception("Failed to accept client");

	GameState gameState = defaultStartGameState();

	while (true)
	{
		std::vector<char> gameStateMsg = tcpMsgGameState(gameState);
		send(playerSockets[0], gameStateMsg.data(), gameStateMsg.size(), 0);
		//send(playerSockets[1], gameStateMsg.data(), gameStateMsg.size(), 0);

		std::vector<char> requestMoveMsg = tcpMsgRequestMove();
		send(playerSockets[(int)gameState.turn], requestMoveMsg.data(), requestMoveMsg.size(), 0);
	}

	CLOSESOCKET(playerSockets[0]);
	CLOSESOCKET(playerSockets[1]);
}

std::vector<char> Server::tcpMsgGameState(const GameState& gameState)
{
	std::vector<char> msg;
	msg.push_back((char)TcpMsgType::GAME_STATE);

	char* gameStatePtr = (char*)&gameState;
	for (size_t i = 0; i < sizeof(gameState); ++i)
		msg.push_back(*(gameStatePtr + i));

	msg[0] = msg.size();

	return msg;
}

std::vector<char> Server::tcpMsgRequestMove()
{
	std::vector<char> msg;
	msg.push_back((char)TcpMsgType::REQUEST_MOVE);
	return msg;
}
