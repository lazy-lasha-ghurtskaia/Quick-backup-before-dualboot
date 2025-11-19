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
} bool sdl_load()
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



typedef struct point3D{
	float inix;
	float iniy;
	float iniz;

	float offx;
	float offy;
	float offz;

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

#define THETA  0.0008726646
//#define FOV 100
volatile float FOV = 250;
void update_proj(point3D* p)
{
	p->dx = (int)(( (p->offx+p->inix) * FOV) / ( (p->offz+p->iniz) + FOV));
	p->dy = (int)(( (p->offy+p->iniy) * FOV) / ( (p->offz+p->iniz) + FOV));
}
void rotate_y(point3D* p)
{
	float finx;
	float finz;
	finx = (p->inix * cos(THETA)) + (p->iniz * sin(THETA));
	finz = (p->inix * -(sin(THETA))) + (p->iniz * cos(THETA));
	p->inix = finx;
	p->iniz = finz;
}
void rotate_ny(point3D* p)
{
	float finx;
	float finz;
	finx = (p->inix * cos(-THETA)) + (p->iniz * sin(-THETA));
	finz = (p->inix * -(sin(-THETA))) + (p->iniz * cos(-THETA));
	p->inix = finx;
	p->iniz = finz;
}

void rotate_x(point3D* p)
{
	float finy;
	float finz;
	finy = (p->iniy * cos(THETA)) + (p->iniz * -(sin(THETA)));
	finz = (p->iniy * sin(THETA)) + (p->iniz * cos(THETA));
	p->iniy = finy;
	p->iniz = finz;
}
void rotate_nx(point3D* p)
{
	float finy;
	float finz;
	finy = (p->iniy * cos(-THETA)) + (p->iniz * -(sin(-THETA)));
	finz = (p->iniy * sin(-THETA)) + (p->iniz * cos(-THETA));
	p->iniy = finy;
	p->iniz = finz;
}


int main(int argc, char **argv)
{
	SDL_Window* gWindow = NULL;
	SDL_Renderer* gRenderer = NULL;

	SDL_Event e;
	bool quit = false;

	Uint64 start, end;
	double elapsed_ms;

	const Uint8* keystate;

	point3D p = {(WINDOW_W/2)-400, (WINDOW_H/2)-50, 0,  0, 0, 200,  0, 0};
	point3D p2 = {WINDOW_W/4, WINDOW_H/4, 0,  0, 0, 200,  0, 0};
	point3D p3 = {0, -100, 0,  0, 0, 225,  0, 0};

	update_proj(&p);
	update_proj(&p2);
	update_proj(&p3);

	if (!sdl_init(&gWindow, &gRenderer)){
		printf("init failure.\n");
	}else{
		render_scene(&gRenderer, p, p2, p3);

		while (!quit){

			keystate = SDL_GetKeyboardState(nullptr);
			if (keystate[SDL_SCANCODE_W]){
				start = SDL_GetPerformanceCounter();

				p.offz+=0.05; p2.offz+=0.05; p3.offz+=0.05;
				update_proj(&p);
				update_proj(&p2);
				update_proj(&p3);
				render_scene(&gRenderer, p, p2, p3);

				end = SDL_GetPerformanceCounter();
				elapsed_ms = (end-start) * 1000.0 / (double)SDL_GetPerformanceFrequency();
				printf("Rotate left took %.3f ms\n", elapsed_ms);
			}
			if (keystate[SDL_SCANCODE_S]){
				p.offz-=0.05; p2.offz-=0.05; p3.offz-=0.05;
				update_proj(&p);
				update_proj(&p2);
				update_proj(&p3);
				render_scene(&gRenderer, p, p2, p3);
			}
			if (keystate[SDL_SCANCODE_D]){
				p.offx+=0.05; p2.offx+=0.05; p3.offx+=0.05;
				update_proj(&p);
				update_proj(&p2);
				update_proj(&p3);
				render_scene(&gRenderer, p, p2, p3);
			}
			if (keystate[SDL_SCANCODE_A]){
				p.offx-=0.05; p2.offx-=0.05; p3.offx-=0.05;
				update_proj(&p);
				update_proj(&p2);
				update_proj(&p3);
				render_scene(&gRenderer, p, p2, p3);
			}
			if (keystate[SDL_SCANCODE_E]){
				p.offy+=0.05; p2.offy+=0.05; p3.offy+=0.05;
				update_proj(&p);
				update_proj(&p2);
				update_proj(&p3);
				render_scene(&gRenderer, p, p2, p3);
			}
			if (keystate[SDL_SCANCODE_Q]){
				p.offy-=0.05; p2.offy-=0.05; p3.offy-=0.05;
				update_proj(&p);
				update_proj(&p2);
				update_proj(&p3);
				render_scene(&gRenderer, p, p2, p3);
			}
			if (keystate[SDL_SCANCODE_LEFT]){
				// first rotate then offset

				start = SDL_GetPerformanceCounter();
				rotate_y(&p);
				rotate_y(&p2);
				rotate_y(&p3);
				update_proj(&p);
				update_proj(&p2);
				update_proj(&p3);
				render_scene(&gRenderer, p, p2, p3);

				end = SDL_GetPerformanceCounter();
				elapsed_ms = (end-start) * 1000.0 / (double)SDL_GetPerformanceFrequency();
				printf("Rotate left took %.3f ms\n", elapsed_ms);
			}
			if (keystate[SDL_SCANCODE_RIGHT]){
				rotate_ny(&p);
				rotate_ny(&p2);
				rotate_ny(&p3);
				update_proj(&p);
				update_proj(&p2);
				update_proj(&p3);
				render_scene(&gRenderer, p, p2, p3);
			}
			if (keystate[SDL_SCANCODE_UP]){
				rotate_x(&p);
				rotate_x(&p2);
				rotate_x(&p3);
				update_proj(&p);
				update_proj(&p2);
				update_proj(&p3);
				render_scene(&gRenderer, p, p2, p3);
			}
			if (keystate[SDL_SCANCODE_DOWN]){
				rotate_nx(&p);
				rotate_nx(&p2);
				rotate_nx(&p3);
				update_proj(&p);
				update_proj(&p2);
				update_proj(&p3);
				render_scene(&gRenderer, p, p2, p3);
			}
			//else if (keystate[SDL_SCANCODE_Z]){
			//	if (FOV<1000)
			//		FOV+=0.01;
			//	update_proj(&p);
			//	update_proj(&p2);
			//	update_proj(&p3);
			//	render_scene(&gRenderer, p, p2, p3);
			//}
			//else if (keystate[SDL_SCANCODE_X]){
			//	if (FOV>0)
			//		FOV-=0.01;
			//	update_proj(&p);
			//	update_proj(&p2);
			//	update_proj(&p3);
			//	render_scene(&gRenderer, p, p2, p3);
			//}
			while(SDL_PollEvent(&e) > 0){
				if (e.type == SDL_QUIT)
					quit = true;
			}
		}
	}

	sdl_exit(&gWindow, &gRenderer);
}
