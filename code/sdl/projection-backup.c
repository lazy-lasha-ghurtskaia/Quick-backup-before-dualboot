#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <math.h>

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


typedef struct point3D{
	float x;
	float y;
	float z;

	int dx;
	int dy;
} point3D;

void render_scene(SDL_Renderer** renderer, point3D p, point3D p2, point3D p3)
{
	SDL_SetRenderDrawColor(*renderer, 0, 0, 0, 0xff);
	SDL_RenderClear(*renderer);

	SDL_SetRenderDrawColor(*renderer, 0xff, 0xff, 0xff, 0xff);
	SDL_RenderDrawLine(*renderer, 320+p.dx, 240-p.dy, 320+p2.dx, 240-p2.dy);
	SDL_RenderDrawLine(*renderer, 320+p2.dx, 240-p2.dy, 320+p3.dx, 240-p3.dy);
	SDL_RenderDrawLine(*renderer, 320+p3.dx, 240-p3.dy, 320+p.dx, 240-p.dy);

	SDL_RenderPresent(*renderer);
}


// base position of one vector on arrow keys
// global variable point3D.
// edit it's values using events
// write a function that will display vectors in that fashion

#define THETA 0.0523 // 3 degrees in radians
#define FOV 250
void update_proj(point3D* p)
{
	p->dx = (int)((p->x * FOV) / (p->z + FOV));
	p->dy = (int)((p->y * FOV) / (p->z + FOV));
}

void rotate_y(point3D* p)
{
	float nx;
	float ny;
	float nz;

	nx = (p->x * cos(THETA)) + (p->z * sin(THETA));
	nz = (p->x * -(sin(THETA))) + (p->z * cos(THETA));

	p->x = nx;
	p->z = nz;
}

int main(int argc, char **argv)
{
	SDL_Window* gWindow = NULL;
	SDL_Renderer* gRenderer = NULL;

	SDL_Event e;
	bool quit = false;

	point3D p = {(WINDOW_W/2)-400, (WINDOW_H/2)-50, 100, 0, 0};
	point3D p2 = {WINDOW_W/4, WINDOW_H/4, 250, 0, 0};
	point3D p3 = {0, -100, 50, 0, 0};

	update_proj(&p);
	update_proj(&p2);
	update_proj(&p3);

	if (!sdl_init(&gWindow, &gRenderer)){
		printf("init failure.\n");
	}else{
		render_scene(&gRenderer, p, p2, p3);
		while (!quit){
			while(SDL_PollEvent(&e) > 0){
				if (e.type == SDL_QUIT)
					quit = true;
				else if (e.type == SDL_KEYDOWN){
					switch(e.key.keysym.sym){
						case(SDLK_q):
							p.z+=10; p2.z+=10; p3.z+=10;
							update_proj(&p);
							update_proj(&p2);
							update_proj(&p3);
							render_scene(&gRenderer, p, p2, p3);
							break;
						case(SDLK_e):
							p.z-=10;
							p2.z-=10;
							p3.z-=10;
							update_proj(&p);
							update_proj(&p2);
							update_proj(&p3);
							render_scene(&gRenderer, p, p2, p3);
							break;
						case(SDLK_d):
							p.x+=10; p2.x+=10; p3.x+=10;
							update_proj(&p);
							update_proj(&p2);
							update_proj(&p3);
							render_scene(&gRenderer, p, p2, p3);
							break;
						case(SDLK_a):
							p.x-=10; p2.x-=10; p3.x-=10;
							update_proj(&p);
							update_proj(&p2);
							update_proj(&p3);
							render_scene(&gRenderer, p, p2, p3);
							break;
						case(SDLK_w):
							p.y+=10; p2.y+=10; p3.y+=10;
							update_proj(&p);
							update_proj(&p2);
							update_proj(&p3);
							render_scene(&gRenderer, p, p2, p3);
							break;
						case(SDLK_s):
							p.y-=10; p2.y-=10; p3.y-=10;
							update_proj(&p);
							update_proj(&p2);
							update_proj(&p3);
							render_scene(&gRenderer, p, p2, p3);
							break;
						case(SDLK_RIGHT):
							rotate_y(&p);
							update_proj(&p);
							render_scene(&gRenderer, p, p2, p3);
							break;
						case(SDLK_LEFT):
							break;
						default:
							break;
					}
				}
			}
		}
	}

	sdl_exit(&gWindow, &gRenderer);
}
