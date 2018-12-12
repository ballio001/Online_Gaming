#include <SDL.h>
#include <SDL_net.h>
#include "Game.h"

#define SDL_reinterpret_cast(type, expression)  reinterpret_cast<type>(expression)

int main(int argc, char ** argv)
{
	Uint32 frameStart, frameTime;
	Game* game = Game::getInstance();

	const char windowTitle[] = "Online Game Technologies";
	game->init(windowTitle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, false);

	while (game->isRunning()) {
		frameStart = SDL_GetTicks();

		game->update();

		frameTime = SDL_GetTicks() - frameStart;

		if (frameTime < DELAY_TIME) {
			SDL_Delay((int)(DELAY_TIME - frameTime));
		}
	}

	game->clean();

	return 0;
}