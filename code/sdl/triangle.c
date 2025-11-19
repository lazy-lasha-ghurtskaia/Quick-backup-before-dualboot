#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
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

bool sdl_load(SDL_Renderer** renderer, SDL_Texture** texture)
{
	bool success = true;
	*texture = load_texture("Apple.png", renderer);
	if (!(*texture))
		success = false;
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


void render_scene(SDL_Renderer** renderer, SDL_Vertex* vertecies, SDL_Texture** texture)
{
	SDL_SetRenderDrawColor(*renderer, 0xff, 0xff, 0xff, 0xff);
	SDL_RenderClear(*renderer);

	SDL_RenderGeometry(*renderer, *texture, vertecies, 3, NULL, 3);

	SDL_RenderPresent(*renderer);
}
int main(int argc, char **argv)
{
	SDL_Window* gWindow = NULL;
	SDL_Renderer* gRenderer = NULL;
	SDL_Texture* gTexture = NULL;

	const Uint8* keystate = NULL;
	float col = 0xff;

	SDL_Event e;
	bool quit = false;

	SDL_Vertex vertices[3] = {
		{ {125,0},	 {0xff, 0xff, 0xff, 0xff}, {0, 0}}, 
		{ {0, 250},	 {0xff, 0xff, 0xff, 0xff}, {0, 1}},
		{ {250, 250},{0xff, 0xff, 0xff, 0xff}, {1, 1}},
	};


	if (!sdl_init(&gWindow, &gRenderer)){
		printf("init failure.\n");
	}else{
		if (sdl_load(&gRenderer, &gTexture)){
			render_scene(&gRenderer, vertices, &gTexture);
			while (!quit){
				keystate = SDL_GetKeyboardState(nullptr);
				if (keystate[SDL_SCANCODE_RIGHT]){
					vertices[0].position.x += 0.1;
				}
				if (keystate[SDL_SCANCODE_LEFT]){
					vertices[0].position.x -= 0.1;
				}
				if (keystate[SDL_SCANCODE_UP]){
					vertices[0].position.y -= 0.1;
				}
				if (keystate[SDL_SCANCODE_DOWN]){
					vertices[0].position.y += 0.1;
				}

				render_scene(&gRenderer, vertices, &gTexture);
				while(SDL_PollEvent(&e) > 0){
					if (e.type == SDL_QUIT)
						quit = true;
				}
			}
		}
	}

	sdl_exit(&gWindow, &gRenderer);
}
