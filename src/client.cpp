#include "client.h"
#include "tcp_messages.h"

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
	_clientThread = std::thread(&Client::nextState, this);
}

Client::~Client()
{
	CLOSESOCKET(_serverSocket);
}

void Client::close()
{
	_closeRequested = true;
	_clientThread.join();
}

void Client::nextState()
{
	bool closing = false;
	while (!closing)
	{
		switch (_state)
		{
			case State::WAITING_FOR_SERVER:
				stateWaitingForServer();
				break;
			case State::SENDING_TO_SERVER_STATE:
				stateSendingToServer();
				break;
			case State::CLOSING:
				closing = true;
				break;
		}
	}
}

void Client::stateWaitingForServer()
{
	std::vector<char> recvMsg;
	recvMsg.resize(128);
	std::future<int> bytesReceived = std::async(recv, _serverSocket, recvMsg.data(), 128, 0);
	while (_state == State::WAITING_FOR_SERVER)
	{
		if (_closeRequested)
		{
			shutdown(_serverSocket, SD_RECEIVE);
			_state = State::CLOSING;
		}
		else if (bytesReceived.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
		{
			TcpMsgType tcpMsgType = (TcpMsgType)recvMsg[0];
			break;
		}
	}
}

void Client::stateSendingToServer()
{

}
