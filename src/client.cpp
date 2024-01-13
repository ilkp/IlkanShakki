#include "client.h"

#include <vector>
#include <future>

Client::Client(const std::string& ip, const std::string& port) :
	_serverSocket(INVALID_SOCKET),
	_state(State::WAITING_FOR_SERVER),
	_closeRequested(false)
{
	addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;

	addrinfo* peerAddress;
	if (getaddrinfo(ip.c_str(), port.c_str(), &hints, &peerAddress))
		throw std::exception("getaddrinfo failed");

	char addressBuffer[100];
	char serviceBuffer[100];
	getnameinfo(
		peerAddress->ai_addr,
		peerAddress->ai_addrlen,
		addressBuffer, sizeof(addressBuffer),
		serviceBuffer, sizeof(serviceBuffer),
		NI_NUMERICHOST);

	_serverSocket = socket(peerAddress->ai_family, peerAddress->ai_socktype, peerAddress->ai_protocol);
	if (!ISVALIDSOCKET(_serverSocket))
		throw std::exception("Failed to create socket");

	if (connect(_serverSocket, peerAddress->ai_addr, peerAddress->ai_addrlen))
		throw std::exception("Failed to connect socket");

	freeaddrinfo(peerAddress);
	_mainLoopThread = std::thread(&Client::mainLoop, this);
}

Client::~Client()
{
	CLOSESOCKET(_serverSocket);
}

void Client::close()
{
	_closeRequested = true;
	_mainLoopThread.join();
}

void Client::mainLoop()
{
	while (!_closeRequested)
	{
		switch (_state)
		{
			case State::WAITING_FOR_SERVER:
				waitingForServerState();
				break;
			case State::SENDING_TO_SERVER_STATE:
				sendingToServerState();
				break;
		}
	}
}

void Client::waitingForServerState()
{
	std::vector<char> recvMsg;
	recvMsg.resize(128);
	std::future<int> bytesReceived = std::async(recv, _serverSocket, recvMsg.data(), 128, 0);
	while (_state == State::WAITING_FOR_SERVER)
	{
		if (_closeRequested)
		{
			break;
		}
		if (bytesReceived.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
		{
			break;
		}
	}
}

void Client::sendingToServerState()
{
}
