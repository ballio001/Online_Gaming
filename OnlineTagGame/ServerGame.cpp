#include "Game.h"
#include <iostream>

using namespace std;

Game *Game::instance = 0;

Game::Game()
{
	portNumber = 1122;
	maxSockets = 30;
}

bool Game::init(const char *title, int xPosition, int yPosition, int height, int width, bool fullScreen)
{
	int flags = SDL_WINDOW_SHOWN;

	if (fullScreen) {
		flags = SDL_WINDOW_FULLSCREEN;
	}

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return false;
	}

	SDLNet_Init();

	/*window = SDL_CreateWindow(title, xPosition, yPosition, height, width, flags);

	if (!window) {
		std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		return false;
	}

	renderer = SDL_CreateRenderer(window, -1, 0);

	if (!renderer) {
		return false;
	}*/

	SDLNet_ResolveHost(&ip, NULL, portNumber);

	// init create game state
	//gameStateMachine = new GameStateMachine();
	//gameStateMachine->changeState(new MenuState());

	//SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

	//create a socket set that will be able to watch up to a given amount of sockets.
	sockets = SDLNet_AllocSocketSet(maxSockets);

	//To create a listening TCP socket on the given port number (server):
	server = SDLNet_TCP_Open(&ip);

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

	// render gamestatemachine
	//gameStateMachine->render();

	SDL_RenderPresent(renderer);
}

void Game::handleEvents()
{
	// handle input
	/*InputHandler *inputHandler = TheInputHandler::getInstance();
	inputHandler->update();*/
}

void Game::update()
{
	//Accept an incoming connection on the server TCPsocket.

	// The input parameter "server" is the server TCPsocket which was previously created by SDLNet_TCP_Open.

	// "server" socket is never connected to a remote host. What you get back is a new "TCPsocket" that is connected to the remote host.

	//This is a non-blocking call, so if no connections are there to be accepted, you will get a NULL TCPsocket and the program will continue going.
	TCPsocket tmpsocket = SDLNet_TCP_Accept(server);
	if (tmpsocket)
	{
		if (playernum < maxSockets)
		{
			SDLNet_TCP_AddSocket(sockets, tmpsocket);
			socketvector.push_back(data(tmpsocket, SDL_GetTicks(), curid));
			playernum++;
			sprintf_s(tmp, "0 %d \n", curid);
			curid++;
			std::cout << "New connection: " << curid << '\n';
		}
		else {
			sprintf_s(tmp, "3 \n");
		}
		SDLNet_TCP_Send(tmpsocket, tmp, strlen(tmp) + 1);
	}

	if (SDLNet_SocketReady(server)) {
		TCPsocket client = SDLNet_TCP_Accept(server);
		if (client) {
			SDLNet_TCP_Send(client, tmp, strlen(tmp) + 1);
		}
	}
	//check for incoming data
	// Check all sockets in the socket set for activity. If a non-zero timeout is given then this function will wait for activity, or else it will wait for timeout milliseconds.
	while (SDLNet_CheckSockets(sockets, 0) > 0)
	{
		for (int i = 0; i < socketvector.size(); i++)
		{
			
			//Check whether a socket has been marked as active.
			if (SDLNet_SocketReady(socketvector[i].socket))
			{
				socketvector[i].timeout = SDL_GetTicks();
				memset(tmp, 0, sizeof(tmp));
				SDLNet_TCP_Recv(socketvector[i].socket, tmp, maxMessageLength);

				int num = tmp[0] - '0';
				int j = 1;
				while (tmp[j] >= '0' && tmp[j] <= '9')
				{
					num *= 10;
					num += tmp[j] - '0';
					j++;
				}

				for (int k = 0; k < socketvector.size(); k++)
				{
					if (k == i)
						continue;
					SDLNet_TCP_Send(socketvector[k].socket, tmp, strlen(tmp) + 1);
				}
				
			}
		}
	}

	// disconnect, timeout
	for (int j = 0; j < socketvector.size(); j++)
		if (SDL_GetTicks() - socketvector[j].timeout > 100000)
		{
			sprintf_s(tmp, "2 %d \n", socketvector[j].id);
			for (int k = 0; k < socketvector.size(); k++)
			{
				SDLNet_TCP_Send(socketvector[k].socket, tmp, strlen(tmp) + 1);
			}
			SDLNet_TCP_DelSocket(sockets, socketvector[j].socket);
			SDLNet_TCP_Close(socketvector[j].socket);
			socketvector.erase(socketvector.begin() + j);
			playernum--;
		}
	SDL_Delay(1);
}

void Game::clean()
{
	running = false;

	// Networking clean
	for (int i = 0; i < socketvector.size(); i++)
		SDLNet_TCP_Close(socketvector[i].socket);
	SDLNet_FreeSocketSet(sockets);
	SDLNet_TCP_Close(server);

	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);

	SDLNet_Quit();
	SDL_Quit();
}