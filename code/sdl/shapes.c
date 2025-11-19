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
			flags = IMG_INIT_PNG;
			if ( !(IMG_Init(flags) | flags) ){
				printf("IMG_Init error: %s\n", IMG_GetError());
				success = false;
			}
		}
	}
	return success;
}

SDL_Texture* load_texture(char *path, SDL_Renderer** renderer)
{
	SDL_Texture* texture = NULL;
	SDL_Surface* surface = NULL;
	surface = IMG_Load(path);
	if (!surface){
		printf("failure to load: %s. IMG_Load error: %s.\n", path, IMG_GetError());
	} else {
		texture = SDL_CreateTextureFromSurface(*renderer, surface);
		if (!texture){
			printf("SDL_CreateTextureFromSurface error %s\n", SDL_GetError());
		}
		SDL_FreeSurface(surface);
	}
	return texture;
}

bool sdl_load()
{
	return true;
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

int y2 = WINDOW_H/2;
int x2 = WINDOW_W/3;
void render_scene(SDL_Renderer** renderer)
{
	SDL_SetRenderDrawColor(*renderer, 0xff, 0xff, 0xff, 0xff);
	SDL_RenderClear(*renderer);

	SDL_Rect rect = {WINDOW_W/4, WINDOW_H/4, WINDOW_W/2, WINDOW_H/2};
	SDL_SetRenderDrawColor(*renderer, 0xff, 0, 0, 0xff);
	SDL_RenderFillRect(*renderer, &rect);

	SDL_SetRenderDrawColor(*renderer, 0, 0xff, 0, 0xff);
	SDL_RenderDrawLine(*renderer, WINDOW_W/2, WINDOW_H/2, x2, y2);

	SDL_RenderPresent(*renderer);
}

int main(int argc, char **argv)
{
	SDL_Window* gWindow = NULL;
	SDL_Renderer* gRenderer = NULL;

	SDL_Event e;
	bool quit = false;

	if (!sdl_init(&gWindow, &gRenderer)){
		printf("init failure.\n");
	}else{
		render_scene(&gRenderer);
		while (!quit){
			while(SDL_PollEvent(&e) > 0){
				if (e.type == SDL_QUIT)
					quit = true;
				else if (e.type == SDL_KEYDOWN){
					switch(e.key.keysym.sym){
						case(SDLK_LEFT):
							x2-=4;
							render_scene(&gRenderer);
							break;
						case(SDLK_RIGHT):
							x2+=4;
							render_scene(&gRenderer);
							break;
						case(SDLK_UP):
							y2-=4;
							render_scene(&gRenderer);
							break;
						case(SDLK_DOWN):
							y2+=4;
							render_scene(&gRenderer);
							break;
						default:
							render_scene(&gRenderer);
							break;
					}
				}
			}
		}
	}

	sdl_exit(&gWindow, &gRenderer);
}
