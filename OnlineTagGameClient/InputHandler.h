#pragma once
#include<SDL.h>
#include<vector>
#include"Vector2D.h"

enum MouseButtons
{
	LEFT = 0,
	MIDDLE = 1,
	RIGHT = 2
};

class InputHandler
{
public:
	static InputHandler* getInstance()
	{
		if (!instance) {
			instance = new InputHandler();
		}

		return instance;
	}

	void update();

	bool getMouseButtonState(int buttonNumber);

	Vector2D* getMousePosition();

	bool isKeyDown(SDL_Scancode key);

private:
	InputHandler();
	~InputHandler() {}

	void onKeyDown();
	void onKeyUp();

	void onMouseMove(SDL_Event &event);
	void onMouseButtonDown(SDL_Event &event);
	void onMouseButtonUp(SDL_Event &event);

	static InputHandler *instance;

	std::vector< std::vector<bool> > buttonStates;
	std::vector<bool> mouseButtonStates;
	Vector2D *mousePosition;

	Uint8 *keyStates;
};

typedef InputHandler TheInputHandler;