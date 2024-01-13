
#include <thread>

#include "game.h"
#include "chess.h"
#include "server.h"
#include "client.h"

Game::Game(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

	#if defined(_WIN32)
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData))
		throw std::exception("Failed to initialize windows socket api");
	#endif

	Server server("5555");
	Client client("127.0.0.1", "5555");
	//Client client2("127.0.0.1", "5555");

	Sleep(10000);
	client.close();
	//client._mainLoopThread.join();
}

Game::~Game()
{
	#if defined(_WIN32)
	WSACleanup();
	#endif
}
