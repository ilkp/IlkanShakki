#pragma once

enum class TcpMsgType : unsigned char
{
	GAME_STATE,
	MOVE,
	REQUEST_MOVE
};