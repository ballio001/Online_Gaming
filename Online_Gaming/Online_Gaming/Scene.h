#pragma once
#include <iostream>
#include "SDL.h"
#include "Identifiers.h"

class SceneManager;
class Scene
{
public:  
	Scene(SceneManager& sceneManager, ContentLoader& contentLoader)
		: m_sceneManager(&sceneManager)
	{
	}
	virtual ~Scene() {};


	//virtual void handleEvents(InputHandler * inputHandler) = 0;
	virtual void update(float deltaTime) = 0;
	virtual void render(SDL_Renderer * renderer) = 0;

	virtual bool onEnter() = 0;
	virtual bool onExit() = 0;

protected:
	SceneManager* m_sceneManager;

};

