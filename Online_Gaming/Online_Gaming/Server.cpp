#include "stdafx.h"
#include "Server.h"

Server::Server()
	: m_running(false)
	, m_currentID(0)
	, m_totalDataSent(0)
	, m_totalDataReceived(0)
	, m_mutex(SDL_CreateMutex())
	, m_packetSize(std::numeric_limits<Uint8>::max())
	, m_rooms(MAX_ROOMS)
	, m_gameStarted(MAX_ROOMS, false)
{
}


Server::~Server()
{
}

bool Server::removeClient(const int & clientID)
{
	//SDL_LockMutex(m_mutex);
	auto itr = m_clients.find(clientID);
	if (itr != m_clients.end())
	{
		PacketData pData;
		pData << (Uint8)PacketType::Disconnect;
		send(clientID, pData);
		std::cout << itr->second.m_clientIP << " : " << itr->second.m_clientPort << " Disconnected" << std::endl;
		leaveRoom(clientID);
		m_clients.erase(itr);
		//SDL_UnlockMutex(m_mutex);
		return true;
	}
//	SDL_UnlockMutex(m_mutex);
	return false;
}
void Server::disconnectAll()
{
	PacketData pData;
	pData << (Uint8)PacketType::Disconnect;
	broadcastToEveryone(pData);
	SDL_LockMutex(m_mutex);
	m_clients.clear();
	clearRooms();
	SDL_UnlockMutex(m_mutex);
}

bool Server::start(ThreadPool * threadpool)
{
	if (SDLNet_Init() != -1)
	{
		m_socket = SDLNet_UDP_Open(SERVERPORT);
		m_packet = SDLNet_AllocPacket(m_packetSize);
		//threadpool->addTask(std::bind(&Server::listen, this));
		m_running = true;
		return true;
	}
	return false;
}

bool Server::stop()
{
	disconnectAll();
	m_running = false;
	SDLNet_UDP_Close(m_socket);
	m_socket = NULL;
	return true;
}

bool Server::isRunning()
{
	return m_running;
}

void Server::update()
{
	m_serverTime = SDL_GetTicks();	// Time since server was started

	//SDL_LockMutex(m_mutex);
	for (auto itr = m_clients.begin(); itr != m_clients.end();)
	{
		Uint32 elapsedTime = m_serverTime - itr->second.m_lastConnectionTime;
		if (elapsedTime > CONNECTION_TIME_INTERVAL)
		{
			if (elapsedTime > CLIENT_TIMEOUT || itr->second.m_connectionRetry > MAX_CONNECTION_RETRIES)
			{
				std::cout << "Client " << itr->first << " has timed out." << std::endl;
				leaveRoom(itr->first);
				itr = m_clients.erase(itr);
				continue;
			}
			if (!itr->second.m_connectionWaiting || (elapsedTime >= CONNECTION_TIME_INTERVAL * (itr->second.m_connectionRetry + 1)))
			{
				PacketData pData;
				pData << (Uint8)PacketType::Heartbeat << m_serverTime;
				send(itr->first, pData);
				if (itr->second.m_connectionRetry == 0)
				{
					itr->second.m_currentConnectionTime = m_serverTime;
				}
				itr->second.m_connectionWaiting = true;
				itr->second.m_connectionRetry++;
				m_totalDataSent += pData.getDataSize();
			}
		}
		++itr;
	}
	//SDL_UnlockMutex(m_mutex);
	listen();
}

bool Server::send(const int & clientID, PacketData & packetData)
{
	//SDL_LockMutex(m_mutex);
	auto itr = m_clients.find(clientID);
	if (itr != m_clients.end())
	{
		int packetSize = packetData.getDataSize();
		if (packetSize > 0)
		{
			m_packet->len = packetSize;
			m_packet->address.host = itr->second.m_clientIP;
			m_packet->address.port = itr->second.m_clientPort;
			memcpy(m_packet->data, packetData.getData(), m_packet->len);
			SDLNet_UDP_Send(m_socket, -1, m_packet);
			m_totalDataSent += packetSize;
			//SDL_UnlockMutex(m_mutex);
			return true;
		}
	}
	//SDL_UnlockMutex(m_mutex);
	return false;
}

void Server::broadcastToRoomOnly(PacketData & packetData, const Uint8 & roomID, const int & ignoreClientID)
{
	//SDL_LockMutex(m_mutex);
	for (auto &client : m_clients)
	{
		if (client.second.m_roomID == roomID && client.first != ignoreClientID)
		{
			int packetSize = packetData.getDataSize();
			if (packetSize > 0)
			{
				m_packet->len = packetSize;
				m_packet->address.host = client.second.m_clientIP;
				m_packet->address.port = client.second.m_clientPort;
				memcpy(m_packet->data, packetData.getData(), m_packet->len);
				SDLNet_UDP_Send(m_socket, -1, m_packet);
				m_totalDataSent += packetSize;
			}
		}
	}
	//SDL_UnlockMutex(m_mutex);
}
void Server::broadcastToLobbyOnly(PacketData & packetData, const Uint8 & roomID, const int & ignoreClientID)
{
	//SDL_LockMutex(m_mutex);
	for (auto &client : m_clients)
	{
		if (client.second.m_roomID == ROOM_OUT_OF_BOUNDS && client.first != ignoreClientID)
		{
			int packetSize = packetData.getDataSize();
			if (packetSize > 0)
			{
				m_packet->len = packetSize;
				m_packet->address.host = client.second.m_clientIP;
				m_packet->address.port = client.second.m_clientPort;
				memcpy(m_packet->data, packetData.getData(), m_packet->len);
				SDLNet_UDP_Send(m_socket, -1, m_packet);
				m_totalDataSent += packetSize;
			}
		}
	}
	//SDL_UnlockMutex(m_mutex);
}
void Server::broadcastToEveryone(PacketData & packetData, const int & ignoreClientID)
{
	//SDL_LockMutex(m_mutex);
	for (auto &client : m_clients)
	{
		if (client.first != ignoreClientID)
		{
			int packetSize = packetData.getDataSize();
			if (packetSize > 0)
			{
				m_packet->len = packetSize;
				m_packet->address.host = client.second.m_clientIP;
				m_packet->address.port = client.second.m_clientPort;
				memcpy(m_packet->data, packetData.getData(), m_packet->len);
				SDLNet_UDP_Send(m_socket, -1, m_packet);
				m_totalDataSent += packetSize;
			}
		}
	}
	//SDL_UnlockMutex(m_mutex);
}
void Server::listen()
{ 
	//std::cout << "Server is listening on port " << SERVERPORT << std::endl;
	//while (m_running)
	//{
		if (SDLNet_UDP_Recv(m_socket, m_packet) > 0)
		{
			PacketData pData;								
			pData.append(m_packet->data, m_packet->len);
			m_totalDataReceived += pData.getDataSize();

			Uint8 packetType;
			pData >> packetType;

			if (packetType >= (Uint8)PacketType::Connect && packetType < (Uint8)PacketType::OutOfBounds)
			{
				if (packetType == (Uint8)PacketType::Heartbeat)
				{
					bool updatedClient = false;
					//SDL_LockMutex(m_mutex);
					for (auto &itr : m_clients) 
					{
						if (!updatedClient && itr.second.m_clientIP == m_packet->address.host && itr.second.m_clientPort == m_packet->address.port)
						{
							itr.second.m_ping = m_serverTime - itr.second.m_currentConnectionTime;
							itr.second.m_lastConnectionTime = m_serverTime;
							itr.second.m_connectionWaiting = false;
							itr.second.m_connectionRetry = 0;
							updatedClient = true;
						}
					}
					//SDL_UnlockMutex(m_mutex);
				}
				else
				{
					packetHandler(m_packet->address.host, m_packet->address.port, packetType, pData);
				}
			} 
		}
	//	SDL_Delay(10);
	//}
}

int Server::addClient(Uint32 & ip, Uint16 & port)
{
	//SDL_LockMutex(m_mutex);
	for (auto &itr : m_clients)
	{
		if (itr.second.m_clientIP == ip && itr.second.m_clientPort == port)
		{
			//SDL_UnlockMutex(m_mutex);
			return -1;
		}
	}
	int id = m_currentID;
	ClientInfo info(ip, port, m_serverTime);
	m_clients.emplace(id, info);
	++m_currentID;
	std::cout << "Client Connected " << std::to_string(ip) << " : " << std::to_string(port) << std::endl;
	//SDL_UnlockMutex(m_mutex);
	return id;
}

int Server::getClientID(const Uint32 & ip, const Uint16 & port)
{
	//SDL_LockMutex(m_mutex);
	for (auto &itr : m_clients)
	{
		if (itr.second.m_clientIP == ip && itr.second.m_clientPort == port)
		{
			//SDL_UnlockMutex(m_mutex);
			return itr.first;
		}
	}
	//SDL_UnlockMutex(m_mutex);
	return -1;
}
Uint8 Server::getClientRoomID(const int & clientID)
{
	Uint8 roomID = std::numeric_limits<Uint8>::max();
	//SDL_LockMutex(m_mutex);
	auto itr = m_clients.find(clientID);
	if (itr != m_clients.end())
	{
		roomID = itr->second.m_roomID;
	//	SDL_UnlockMutex(m_mutex);
		return roomID;
	}
	//SDL_UnlockMutex(m_mutex);
	return roomID;
}
void Server::joinRoom(int clientID, Uint8 roomID)
{
	PacketData pData;
	std::string message;
	//SDL_LockMutex(m_mutex);
	if (m_rooms[roomID].size() < MAX_CLIENTS_PER_ROOM && roomID != ROOM_OUT_OF_BOUNDS && !m_gameStarted[roomID])
	{
		auto itr = m_clients.find(clientID);
		if (itr != m_clients.end())
		{
			itr->second.m_roomID = roomID;
			m_rooms[roomID].push_back(clientID);
			pData << (Uint8)PacketType::ClientJoinedRoom;
			broadcastToRoomOnly(pData, roomID, clientID);


			pData.clear();

			pData << (Uint8)PacketType::EnterRoom;
			send(clientID, pData);
			updateRoomData();
		}
	}
	else
	{
		message = "Room is full " + roomID;
		pData << (Uint8)PacketType::Message << message;
		send(clientID, pData);
	}

	//SDL_UnlockMutex(m_mutex);
}
void Server::leaveRoom(int clientID)
{
	//SDL_LockMutex(m_mutex);
	auto itr = m_clients.find(clientID);
	if (itr != m_clients.end())
	{
		Uint8 roomID = itr->second.m_roomID;
		if (roomID != ROOM_OUT_OF_BOUNDS)
		{
			m_rooms[roomID].erase(std::remove(m_rooms[roomID].begin(), m_rooms[roomID].end(), clientID), m_rooms[roomID].end());
			itr->second.m_roomID = ROOM_OUT_OF_BOUNDS;
			if (m_rooms[roomID].size() == 0 && m_gameStarted[roomID])
			{
				m_rooms[roomID].clear();
				m_gameStarted[roomID] = false;
			}
			updateRoomData();
		}
	}
	//SDL_UnlockMutex(m_mutex);
}
void Server::clearRooms()
{
	for (size_t i = 0; i < m_rooms.size(); i++)
	{
		m_rooms[i].clear();
	}
}
void Server::sendRoomData(int clientID)
{
	PacketData pData;
	pData << (Uint8)PacketType::Connect << clientID << m_serverTime;
	for (int i = 0; i < m_rooms.size(); i++)
	{
		if (!m_gameStarted[i])
		{
			Uint8 roomSpacesLeft = MAX_CLIENTS_PER_ROOM - m_rooms[i].size();
			pData << roomSpacesLeft;
		}
		else
		{
			pData << MAX_CLIENTS_PER_ROOM;
		}
	}
	send(clientID, pData);
}

void Server::updateRoomData()
{
	PacketData pData;
	pData << (Uint8)PacketType::UpdateRoomData;
	for (int i = 0; i < m_rooms.size(); i++)
	{
		if (!m_gameStarted[i])
		{
			Uint8 roomSpacesLeft = MAX_CLIENTS_PER_ROOM - m_rooms[i].size();
			pData << roomSpacesLeft;
		}
		else
		{
			pData << MAX_CLIENTS_PER_ROOM;
		}
	}
	broadcastToLobbyOnly(pData, ROOM_OUT_OF_BOUNDS);
}

std::vector<int> Server::getClientDataFromRoomAndStartGame(Uint8 roomID)
{
	m_gameStarted[roomID] = true;
	return m_rooms[roomID];
}

std::string Server::getRoomData()
{
	std::string list;
	std::string delimiter = "--------------------------------------";
	list = delimiter;
	list += "\nRoom Details\n" + delimiter + "\n";
	for (size_t i = 0; i < m_rooms.size(); i++)
	{
		list += "Room [" + std::to_string(i) + "] has " + std::to_string(m_rooms[i].size()) + " Clients\n";
	}
	list += delimiter;
	return list;
}

std::string Server::getClientList()
{
	std::string list;
	std::string delimiter = "--------------------------------------";
	list = delimiter;
	list += "\nID\tIP : Port\t\tPing\n" + delimiter + "\n";
	for (auto &itr : m_clients)
	{
		list += std::to_string(itr.first) + "\t" +std::to_string(itr.second.m_clientIP) + ":" + std::to_string(itr.second.m_clientPort) + "\t\t" + std::to_string(itr.second.m_ping) + "ms.\n";
	}
	list += delimiter;
	list += '\n';
	list += "Total data sent: " + std::to_string(m_totalDataSent / 1000) + "kB. Total data received: " + std::to_string(m_totalDataReceived / 1000) + "kB";
	return list;
}

Uint8 Server::getMaxClientsPerRoom()
{
	return MAX_CLIENTS_PER_ROOM;
}

int Server::getServerTime()
{
	return m_serverTime;
}
