#include "pch.h"
#include "Game.h"
#include "InputHandler.h"
#include <iostream>
#include "PacketStream.h"

using namespace std;

Game *Game::instance = 0;
SDLNet_SocketSet socketSet = SDLNet_AllocSocketSet(1);

Game::Game()
{
	portNumber = 1555;
	maxSockets = 30;
}

bool Game::init(const char *title, int xPosition, int yPosition, int height, int width, bool fullScreen)
{
	int flags = SDL_WINDOW_SHOWN;
	this->height = height;
	this->width = width;
	if (fullScreen) {
		flags = SDL_WINDOW_FULLSCREEN;
	}

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return false;
	}

	SDLNet_Init();

	window = SDL_CreateWindow(title, xPosition, yPosition, height, width, flags);

	if (!window) {
		std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		return false;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (!renderer) {
		return false;
	}
	//SDLNet_ResolveHost(&ip, "localhost", portNumber);

	//server = SDLNet_ResolveIP(&ip);
	//client = SDLNet_TCP_Open(&ip);

	// Create the socket set with enough space to store our desired number of connections (i.e. sockets)
	if (socketSet == NULL)
	{
		cout << "Failed to allocate the socket set: " << SDLNet_GetError() << endl;
		exit(-1); // Quit!
	}
	else
	{
		cout << "Successfully allocated socket set." << endl;
	}

	// Try to resolve the host. If successful, this places the connection details in the serverIP object
	int hostResolved = SDLNet_ResolveHost(&ip, "localhost", 1122);

	if (hostResolved == -1)
	{
		cout << "Failed to resolve the server hostname: " << SDLNet_GetError() << "\nContinuing...\n";
	}
	else // If we successfully resolved the host then output the details
	{
		// Get our IP address in proper dot-quad format by breaking up the 32-bit unsigned host address and splitting it into an array of four 8-bit unsigned numbers...
		Uint8 * dotQuad = (Uint8*)&ip.host;

		//... and then outputting them cast to integers. Then read the last 16 bits of the serverIP object to get the port number
		cout << "Successfully resolved host to IP: " << (unsigned short)dotQuad[0] << "." << (unsigned short)dotQuad[1] << "." << (unsigned short)dotQuad[2] << "." << (unsigned short)dotQuad[3];
		cout << " port " << SDLNet_Read16(&ip.port) << endl << endl;
	}

	// Try to resolve the IP of the server, just for kicks
	if ((server = SDLNet_ResolveIP(&ip)) == NULL)
	{
		cout << "Failed to resolve the server IP address: " << SDLNet_GetError() << endl;
	}
	else
	{
		cout << "Successfully resolved IP to host: " << server << endl;
	}

	// Flag to keep track of when to disconnect and finish up. We initially set it so that we CANNOT connect, and only change this to false when we got an "OK" response from the server
	bool shutdownClient = true;

	// Try to open a connection to the server and quit out if we can't connect
	client = SDLNet_TCP_Open(&ip);
	if (!client)
	{
		cout << "Failed to open socket to server: " << SDLNet_GetError() << "\n";
		exit(-1);
	}
	else // If we successfully opened a connection then check for the server response to our connection
	{
		SDLNet_TCP_AddSocket(socketSet, client);
	}
	// Wait for up to five seconds for a response from the server
	// Note: If we don't check the socket set and WAIT for the response, we'll be checking before the server can respond, and it'll look as if the server sent us nothing back
	int activeSockets = SDLNet_CheckSockets(socketSet, 5000);

	cout << "There are " << activeSockets << " socket(s) with data on them at the moment." << endl;

	if (SDLNet_SocketReady(client) != 0)
	{
		//socketvector[i].timeout = SDL_GetTicks();
		memset(tmp, 0, sizeof(tmp));
		SDLNet_TCP_Recv(client, tmp, maxMessageLength);
		int num = tmp[0] - '0';
		int j = 1;
		while (tmp[j] >= '0' && tmp[j] <= '9')
		{
			num *= 10;
			num += tmp[j] - '0';
			j++;
		}
		if (num == 0) {
			int tmpvar;
			sscanf_s(tmp, "0 %d", &tmpvar);
			if (tmpvar == 0) {
				you = new Dot(true);
				other = new Dot(false);
			}else {
				you = new Dot(false);
				other = new Dot(true);
			}
			you->Init(renderer);
			other->Init(renderer);
		}
	}

	// init create game state
	//gameStateMachine = new GameStateMachine();
	//gameStateMachine->changeState(new MenuState());

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	// init inputhandler and texture manager (singletons)
	//TheTextureManager::getInstance()->load("assets/animate-alpha.png", "animate", renderer);

	// init game objects (players etc)
	//Player *player = new Player(new LoaderParams(100, 100, 128, 82, "animate"));
	//Enemy *enemy = new Enemy(new LoaderParams(300, 300, 128, 82, "animate"));

	// add gameobjects to the list
	//gameObjects.push_back(player);
	//gameObjects.push_back(enemy);

	running = true;
	return true;
}

void Game::render()
{
	SDL_RenderClear(renderer);
	you->render(renderer);
	other->render(renderer);
	// render gamestatemachine
	//gameStateMachine->render();

	SDL_RenderPresent(renderer);
}

void Game::handleEvents()
{
	// handle input
	//InputHandler *inputHandler = TheInputHandler::getInstance();
	//inputHandler->update();
	SDL_Event e; 
	if (SDL_PollEvent(&e) != 0) {
		you->handleEvent(e);
	}
}

void Game::update()
{
	//Accept an incoming connection on the server TCPsocket.

	// The input parameter "server" is the server TCPsocket which was previously created by SDLNet_TCP_Open.

	// "server" socket is never connected to a remote host. What you get back is a new "TCPsocket" that is connected to the remote host.


	//check for incoming data
	// Check if we got a response from the server
	SDLNet_CheckSockets(socketSet, 0);
	if (SDLNet_SocketReady(client) != 0)
	{
		//socketvector[i].timeout = SDL_GetTicks();
		memset(tmp, 0, sizeof(tmp));
		SDLNet_TCP_Recv(client, tmp, maxMessageLength);
		cout << "Got the following from server: " << tmp << endl;
		int num = tmp[0] - '0';
		int j = 1;
		while (tmp[j] >= '0' && tmp[j] <= '9')
		{
			num *= 10;
			num += tmp[j] - '0';
			j++;
		}
		PacketStream packetStream;
		packetStream.fromCharArray(tmp);
		int x, y;
		packetStream.readInt(x);
		packetStream.readInt(y);
		other->SetPosition(x, y);
		if (num == 1)
		{

		}
	}
	you->move(width, height);
	string tempstr = you->GetPosAsString();
	//cout << tempstr << endl;
	//cout << tempstr.c_str() << endl;
	PacketStream packetStream;
	packetStream.writeInt(you->GetCenterX());
	packetStream.writeInt(you->GetCenterY());
	//sprintf_s(tmp, "1 %s \n", tempstr.c_str());
	packetStream.toCharArray(tmp);
	SDLNet_TCP_Send(client, tmp, maxMessageLength);
}

void Game::clean()
{
	running = false;

	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);

	SDLNet_Quit();
	SDL_Quit();
}