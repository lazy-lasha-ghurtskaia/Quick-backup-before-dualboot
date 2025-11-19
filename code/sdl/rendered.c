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

bool sdl_load(SDL_Renderer** renderer, SDL_Texture** texture, SDL_Texture** texture2)
{
	bool success = true;
	*texture = load_texture("Apple.png", renderer);
	*texture2 = load_texture("Banana-Single.png", renderer);
	if (!(*texture) | !(*texture2))
		success = false;
	return success;
}

void sdl_exit(SDL_Window** window, SDL_Renderer** renderer, SDL_Texture** texture)
{
	SDL_DestroyTexture(*texture);
	*texture = NULL;

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
	SDL_Texture* gTexture = NULL;
	SDL_Texture* gTexture2 = NULL;

	SDL_Event e;
	bool quit = false;

	if (!sdl_init(&gWindow, &gRenderer)){
		printf("init failure.\n");
	}else{
		if (!sdl_load(&gRenderer, &gTexture, &gTexture2)){
			printf("resource loading failure.\n");
		}else{
			SDL_RenderClear(gRenderer);
			SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);
			SDL_RenderPresent(gRenderer);
			while (!quit){

				while(SDL_PollEvent(&e) > 0){
					if (e.type == SDL_QUIT)
						quit = true;
					else if (e.type == SDL_KEYDOWN){
						SDL_RenderClear(gRenderer);
						switch(e.key.keysym.sym){
							case(SDLK_n):
								SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);
								break;
							default:
								SDL_RenderCopy(gRenderer, gTexture2, NULL, NULL);
								break;
						}
						SDL_RenderPresent(gRenderer);
					}
				}

			}
		}
	}

	sdl_exit(&gWindow, &gRenderer, &gTexture);
}
