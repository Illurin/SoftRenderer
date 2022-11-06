#pragma once
#include "Core/Rasterization/Pipeline.h"
#include "SDL/SDL.h"

class Renderer {
public:
	Renderer(SDL_Window* window);
	void Present(Pipeline* pipeline)const;

private:
	int width, height;
	SDL_Renderer* renderer;
};