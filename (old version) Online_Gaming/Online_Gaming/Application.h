#pragma once
#include <iostream>
#include <string>
#include "Server.h"
#include "ThreadPool.h"
#include "PacketData.h"
class Application
{
public:
	Application();
	~Application();

	void commandLine();
	void handlePacket(Uint32& ip, Uint16& port, Uint8& packetType, PacketData& packetData);
	void run();

private:
	// Variables
	/////////////////////////
	bool m_quit;
	unsigned int m_lastTime;
	Uint32 m_time;
	Server m_server;
	ThreadPool * m_threadPool;
};

