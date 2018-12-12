#pragma once
#include <SDL.h>
#include <SDL_net.h>
#include <iostream>
#include <vector>
#include <cstring>
#include "Dot.h"

using namespace std;

//Screen dimension constants
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

const int FPS = 60;
const int DELAY_TIME = 1000.0f / FPS;

class Game
{
public:
	static Game *getInstance()
	{
		if (!instance) {
			instance = new Game();
		}

		return instance;
	}

	bool init(const char *title, int xPosition, int yPosition, int height, int width, bool fullScreen);

	void render();
	void update();
	void handleEvents();
	void clean();

	bool isRunning() { return running; }

	SDL_Renderer *getRenderer() const { return renderer; }
private:
	static Game *instance;
	bool running;
	SDL_Window* window;
	SDL_Renderer* renderer;
	int height, width;

	Dot *you;
	Dot *other;
	int curid = 0;
	int playernum = 0;

	IPaddress ip;
	int portNumber = 0;

	struct data {
		TCPsocket socket;
		Uint32 timeout;
		int id; // player/client ID
		data(TCPsocket sock, Uint32 t, int i) :socket(sock), timeout(t), id(i) {}
	};

	vector<data> socketvector;

	static const int maxMessageLength = 1400;
	char tmp[maxMessageLength];

	int maxSockets = 0;

	//create a socket set that will be able to watch up to a given amount of sockets.
	SDLNet_SocketSet sockets;

	//To create a listening TCP socket on the given port number (server):
	TCPsocket client;
	const char* server;
	
	Game();
	~Game() {}
};