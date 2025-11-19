#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <unistd.h>

int WINDOW_WIDTH = 640;
int WINDOW_HEIGHT = 480;

bool sdl_init(SDL_Window** gWindow, SDL_Surface** gWSurface)
{
	bool success = true;

	if (SDL_Init(SDL_INIT_VIDEO) < 0){
		printf("SDL_Init error: %s\n", SDL_GetError());
		success = false;
	}
	else 
	{
		*gWindow = SDL_CreateWindow("Shitty program",
		    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
		if (!(*gWindow))
		{
			printf("SDL_CreateWindow error: %s\n", SDL_GetError());
			success = false;
		}
		else 
		{
			int flags = IMG_INIT_PNG;
			if ( !(IMG_Init( flags ) & flags) )
			{
				printf("SDL_Init error: %s\n", IMG_GetError());
				success = false;
			} 
			else 
			{
				*gWSurface = SDL_GetWindowSurface( *gWindow );
			}
		}
	}
	return success;
}

SDL_Surface* get_surface(char *path, SDL_Surface** gWSurface)
{
	SDL_Surface* temp = NULL;
	SDL_Surface* converted = NULL;
	temp = IMG_Load(path);
	if (!temp){
		printf("get_surface error %s\n", IMG_GetError());
	} else {
		converted = SDL_ConvertSurface(temp, (*gWSurface)->format, 0);
		if (!converted){
			printf("sdl_convertSurface error %s\n", SDL_GetError());
		} 	
		SDL_FreeSurface(temp);
	}
	return converted;
}

bool init_resources(SDL_Surface** gImage, SDL_Surface** gAnother, SDL_Surface** gWSurface)
{
	bool success = true;

	*gImage = get_surface("Apple.png", gWSurface);
	*gAnother = get_surface("Banana-Single.png", gWSurface);

	if (!(*gImage) | !(*gAnother))
		success = false;
	return success;
}
void sdl_quit(SDL_Window** gWindow, SDL_Surface** gImage)
{
	SDL_FreeSurface(*gImage);
	SDL_DestroyWindow( *gWindow );
	SDL_Quit();
}

int main(int argc, char **argv)
{
	SDL_Window* gWindow = NULL;
	SDL_Surface* gWSurface = NULL;
	SDL_Surface* gImage = NULL;
	SDL_Surface* gAnother = NULL;

	SDL_Event e;
	bool quit = false;

	if (!sdl_init(&gWindow, &gWSurface)) {
		exit(0);
	} else {
		if (!init_resources(&gImage, &gAnother, &gWSurface)) {
			exit(0);
		} else {
			SDL_Rect rect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};

			SDL_BlitScaled(gImage, 0, gWSurface, &rect);
			SDL_UpdateWindowSurface( gWindow );

			while (!quit) {
				while (SDL_PollEvent(&e) > 0) {
					if (e.type == SDL_QUIT)
						quit = true;
					else if (e.type == SDL_KEYDOWN){
						switch(e.key.keysym.sym){
							case(SDLK_PAUSE):
								SDL_BlitScaled(gAnother, 0, gWSurface, &rect);
								break;
							default:
								SDL_BlitScaled(gImage, 0, gWSurface, &rect);
								break;
						}
					}
					SDL_UpdateWindowSurface( gWindow );
				}
			}

		}
	}
	sdl_quit(&gWindow, &gImage);
}
