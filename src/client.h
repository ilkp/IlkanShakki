#pragma once
#include "socket.h"

#include <thread>
#include <atomic>

class Client
{
public:
	Client(const std::string& ip, const std::string& port);
	Client(const Client& other) = delete;
	Client(Client&& other) = delete;
	Client& operator=(const Client& other) = delete;
	Client& operator=(Client&& other) = delete;
	~Client();

	void close();
	std::thread _clientThread;

private:
	enum class State
	{
		WAITING_FOR_SERVER,
		SENDING_TO_SERVER_STATE,
		CLOSING
	};

	State _state;
	SOCKET _serverSocket;
	std::atomic_bool _closeRequested;

	void nextState();
	void stateWaitingForServer();
	void stateSendingToServer();
};