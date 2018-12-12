#include "stdafx.h"
#include "Application.h"

Application::Application()
	: m_quit(false)
	, m_threadPool(new ThreadPool)
{
}

Application::~Application()
{
}

void Application::commandLine()
{
	while (m_server.isRunning())
	{
		std::string str;
		std::getline(std::cin, str);
		if (str == "lc")
		{
			std::cout << m_server.getClientList() << std::endl;
		}
		else if (str == "lr")
		{
			std::cout << m_server.getRoomData() << std::endl;
	
		}
		else if (str == "dc")
		{
			m_server.disconnectAll();
			std::cout << "Disconnected All Clients" << std::endl;
		}
		else if (str == "q")
		{
			m_server.stop();
			std::cout << "Server Stopped" << std::endl;
		}
	}
}
void Application::handlePacket(Uint32& ip, Uint16& port, Uint8& packetType, PacketData& packetData)
{
	int clientID = m_server.getClientID(ip, port);
	if (clientID >= 0)
	{
		if (packetType == (Uint8)PacketType::Disconnect)
		{
			m_server.removeClient(clientID);
		}
		else if (packetType == (Uint8)PacketType::Message || packetType == (Uint8)PacketType::Update)
		{
			int roomID = m_server.getClientRoomID(clientID);
			if (roomID != std::numeric_limits<Uint8>::max())
			{
				m_server.broadcastToRoomOnly(packetData, roomID, clientID);
			}
		}
		else if (packetType == (Uint8)PacketType::StartGame)
		{
			int roomID = m_server.getClientRoomID(clientID);
			if (roomID != std::numeric_limits<Uint8>::max())
			{
				PacketData pData;

				std::vector<int> ids = m_server.getClientDataFromRoomAndStartGame(roomID);
				int sizeOfRoom = ids.size();
				pData << (Uint8)PacketType::StartGame << m_server.getServerTime() << sizeOfRoom;
				for (int i = 0; i < sizeOfRoom; i++)
				{
					pData << ids[i];
				}
				m_server.broadcastToRoomOnly(pData, roomID);
			}
		}
		else if (packetType == (Uint8)PacketType::EnterRoom)
		{
			Uint8 roomID;
			packetData >> roomID;
			m_server.joinRoom(clientID, roomID);
		}
		else if (packetType == (Uint8)PacketType::LeaveRoom)
		{

				m_server.leaveRoom(clientID);
		}
	}
	else if (packetType == (Uint8)PacketType::Connect)
	{
		int clientID = m_server.addClient(ip, port);
		m_server.sendRoomData(clientID);
	}
}


void Application::run()
{
	m_server.setupHandler(&Application::handlePacket, this);
	if(m_server.start(m_threadPool))
	{
		m_threadPool->addTask(std::bind(&Application::commandLine, this));
		while (m_server.isRunning())
		{
			m_server.update();
		}
	}
}

