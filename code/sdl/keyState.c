#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

#define WINDOW_W 640
#define WINDOW_H 480

bool sdl_init(SDL_Window** window, SDL_Renderer** renderer)
{
	int flags;
	bool success = true;
	*window = SDL_CreateWindow("Shitty window", 
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED, 
			WINDOW_W, WINDOW_H, SDL_WINDOW_RESIZABLE);
	if (!(*window))	{
		printf("CreateWindow error: %s\n", SDL_GetError());
		success = false;
	} else {
		*renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
		if (!(*renderer)){
			printf("CreateRenderer error: %s\n", SDL_GetError());
			success = false;
		} else {
			SDL_SetRenderDrawColor(*renderer, 
					0xFF, 0xFF, 0xFF, 0xFF);
		}
	}
	return success;
}

void sdl_exit(SDL_Window** window, SDL_Renderer** renderer)
{
	SDL_DestroyRenderer(*renderer);
	SDL_DestroyWindow(*window);
	*renderer = NULL;
	*window = NULL;

	IMG_Quit();
	SDL_Quit();
}

int main(int argc, char **argv)
{
	SDL_Window* gWindow = NULL;
	SDL_Renderer* gRenderer = NULL;

	float x = (WINDOW_W/2)+50; float y = WINDOW_H/2;
	SDL_Event e;
	bool quit = false;

	if (!sdl_init(&gWindow, &gRenderer)){
		printf("init failure.\n");
	}else{
		SDL_RenderClear(gRenderer);
		SDL_RenderPresent(gRenderer);
		while (!quit){
			SDL_SetRenderDrawColor(gRenderer, 0xff, 0xff, 0xff, 0xff);
			SDL_RenderClear(gRenderer);
			const Uint8* keystate = SDL_GetKeyboardState(nullptr);
			if (keystate[SDL_SCANCODE_LEFT]){
				x-= 0.03;
				SDL_SetRenderDrawColor(gRenderer, 0xff, 0, 0, 0xff);
				SDL_RenderDrawLine(gRenderer, 0, 0, x, y);
				SDL_RenderPresent(gRenderer);
				printf("< x = %f\n", x);
			}
			else if (keystate[SDL_SCANCODE_RIGHT]){
				x+= 0.03;
				SDL_SetRenderDrawColor(gRenderer, 0xff, 0, 0, 0xff);
				SDL_RenderDrawLine(gRenderer, 0, 0, x, y);
				SDL_RenderPresent(gRenderer);
				printf("> x = %f\n", x);
			}
			if (keystate[SDL_SCANCODE_UP]){
				y-= 0.03;
				SDL_SetRenderDrawColor(gRenderer, 0xff, 0, 0, 0xff);
				SDL_RenderDrawLine(gRenderer, 0, 0, x, y);
				SDL_RenderPresent(gRenderer);
				printf("v y = %f\n", x);
			}
			else if (keystate[SDL_SCANCODE_DOWN]){
				y+= 0.03;
				SDL_SetRenderDrawColor(gRenderer, 0xff, 0, 0, 0xff);
				SDL_RenderDrawLine(gRenderer, 0, 0, x, y);
				SDL_RenderPresent(gRenderer);
				printf("^ y = %f\n", x);
			}

			while(SDL_PollEvent(&e) > 0){
				if (e.type == SDL_QUIT)
					quit = true;
			}

		}
	}

	sdl_exit(&gWindow, &gRenderer);
}
