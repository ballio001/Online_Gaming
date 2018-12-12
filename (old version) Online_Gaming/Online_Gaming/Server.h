#pragma once
#include <iostream>
#include <string>
#include <functional>
#include <unordered_map>
#include "SDL_net.h"
#include "PacketData.h"
#include "ThreadPool.h"
#include "Debug.h"

struct ClientInfo
{
	ClientInfo(const Uint32 & ip, const Uint16 & port, const Uint32 serverTime)
		: m_clientIP(ip)
		, m_clientPort(port)
		, m_lastConnectionTime(serverTime)
		, m_currentConnectionTime(serverTime)
		, m_connectionWaiting(false)
		, m_connectionRetry(0)
		, m_ping(0)
		, m_roomID(std::numeric_limits<Uint8>::max())
	{}

	Uint32 m_clientIP;
	Uint16 m_clientPort;
	Uint32 m_lastConnectionTime;
	Uint32 m_currentConnectionTime;
	bool m_connectionWaiting;
	Uint8 m_connectionRetry;
	Uint16 m_ping;
	Uint8 m_roomID;
};

class Server
{
public:
	Server();
	~Server();

	template<class T>
	void setupHandler(void(T::*handlePacket)(Uint32&, Uint16&, Uint8& , PacketData&), T* instance)
	{
		packetHandler = std::bind(handlePacket, instance, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
	}
	std::function<void(Uint32&, Uint16&, Uint8&, PacketData&)> packetHandler;

	void update();

	bool send(const int & clientID, PacketData & packet);
	void broadcastToRoomOnly(PacketData& packetData, const Uint8 & roomID, const int& ignoreClientID = -1);
	void broadcastToLobbyOnly(PacketData& packetData, const Uint8 & roomID, const int& ignoreClientID = -1);
	void broadcastToEveryone(PacketData& packetData, const int& ignoreClientID = -1);
	void listen();

	int addClient(Uint32 & ip, Uint16 & port);
	int getClientID(const Uint32 & ip, const Uint16 & port);
	Uint8 getClientRoomID(const int & clientID);
	bool removeClient(const int& clientID);


	void disconnectAll();
	bool start(ThreadPool * threadpool);
	bool stop();
	bool isRunning();


	void joinRoom(int clientID, Uint8 roomID);
	void leaveRoom(int clientID);
	void clearRooms();
	void sendRoomData(int clientID);
	void updateRoomData();

	std::vector<int> getClientDataFromRoomAndStartGame(Uint8 roomID);
	std::string getRoomData();
	std::string getClientList();
	Uint8 getMaxClientsPerRoom();
	int getServerTime();

private:
	const Uint8 ROOM_OUT_OF_BOUNDS = 255;
	const Uint8 MAX_ROOMS = 3;
	const Uint8 MAX_CLIENTS_PER_ROOM = 5;
	const int CLIENT_TIMEOUT = 10000;
	const int CONNECTION_TIME_INTERVAL = 1000;
	const int MAX_CONNECTION_RETRIES = 3;
	const unsigned short SERVERPORT = 5228;

	bool m_running;
	int m_currentID;
	int m_totalDataSent;
	int m_totalDataReceived;
	int m_mutexStatus;
	Uint32 m_serverTime;

	SDL_mutex * m_mutex;
	Uint8 m_packetSize;

	UDPsocket m_socket;
	UDPpacket * m_packet;
	std::unordered_map<int, ClientInfo> m_clients;
	std::vector<std::vector<int>> m_rooms;
	std::vector<bool> m_gameStarted;
};

