#include "socket.h"

SOCKET listenSocket(const std::string& port)
{
	addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	addrinfo* bindAddress;
	getaddrinfo(0, port.c_str(), &hints, &bindAddress);

	SOCKET result = INVALID_SOCKET;
	result = socket(bindAddress->ai_family, bindAddress->ai_socktype, bindAddress->ai_protocol);
	if (!ISVALIDSOCKET(result))
	{
		throw std::exception("Failed to create socket");
	}

	if (bind(result, bindAddress->ai_addr, bindAddress->ai_addrlen))
	{
		throw std::exception("Failed to bind socket");
	}

	return result;
}

SOCKET connectSocket(const std::string& ip, const std::string& port)
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

	SOCKET peerSocket = socket(peerAddress->ai_family, peerAddress->ai_socktype, peerAddress->ai_protocol);
	if (!ISVALIDSOCKET(peerSocket))
		throw std::exception("Failed to create socket");

	if (connect(peerSocket, peerAddress->ai_addr, peerAddress->ai_addrlen))
		throw std::exception("Failed to connect socket");

	freeaddrinfo(peerAddress);
	return peerSocket;
}
