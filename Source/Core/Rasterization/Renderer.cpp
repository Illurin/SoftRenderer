#include "Renderer.h"

Renderer::Renderer(SDL_Window* window) {
	SDL_GetWindowSize(window, &width, &height);
	renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED);
}

void Renderer::Present(Pipeline* pipeline)const {
	SDL_Point point;
	for (point.y = 0; point.y < height; point.y++) {
		for (point.x = 0; point.x < width; point.x++) {
			Vector3f color = pipeline->ReadFramebuffer(point.x, point.y);
			SDL_SetRenderDrawColor(renderer, (Uint8)(color.x * 255), (Uint8)(color.y * 255), (Uint8)(color.z * 255), SDL_ALPHA_OPAQUE);
			SDL_RenderDrawPoints(renderer, &point, 1);
		}
	}
	SDL_RenderPresent(renderer);
}