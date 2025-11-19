#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>

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
	*texture = load_texture("checker.png", renderer);
	if (!(*texture))
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

typedef struct point3D{
	float inix;
	float iniy;
	float iniz;

	float offx;
	float offy;
	float offz;

	SDL_Vertex pt;
} point3D;

void render_scene(SDL_Renderer** renderer, SDL_Texture** texture, point3D p, point3D p2, point3D p3, point3D p4)
{
	SDL_Vertex vertex1[4] = {p.pt, p2.pt, p3.pt};
	SDL_Vertex vertex2[4] = {p.pt, p3.pt, p4.pt};

	SDL_SetRenderDrawColor(*renderer, 0, 0, 0, 0xff);
	SDL_RenderClear(*renderer);

	SDL_RenderGeometry(*renderer, *texture, vertex1, 3, NULL, 0);
	SDL_RenderGeometry(*renderer, *texture, vertex2, 3, NULL, 0);

	SDL_SetRenderDrawColor(*renderer, 0xff, 0xff, 0, 0xff);
	SDL_RenderDrawLine(*renderer, WINDOW_W/2, 0, WINDOW_W/2, WINDOW_H);
	SDL_SetRenderDrawColor(*renderer, 0, 0xff, 0xff, 0xff);
	SDL_RenderDrawLine(*renderer, 0, WINDOW_H/2, WINDOW_W, WINDOW_H/2);

	SDL_RenderPresent(*renderer);
}

#define THETA  0.0008726646
//#define FOV 100
volatile float FOV = 1000;
void update_proj(point3D* p)
{
	p->pt.position.x = ( ((p->offx+p->inix) * FOV) / ((p->offz+p->iniz) + FOV) ) + (WINDOW_W/2);
	p->pt.position.y = (WINDOW_H/2) - ( ((p->offy+p->iniy)*FOV) / ((p->offz+p->iniz)+FOV) );
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
	SDL_Texture* gTexture = NULL;

	SDL_Event e;
	bool quit = false;
	const Uint8* keystate;

	point3D p = {200, -200, 0,    0, 0, 0,  
		{ {0, 0}, {255,255,255,255}, {-1,-1}, },	 
	};
	point3D p2 = {-200, -200, 0,    0, 0, 0,  
		{ {0, 0}, {255,255,255,255}, {-1,1}, },	 
	};
	point3D p3 = {-200, 200, 0,    0, 0, 0,  
		{ {0, 0}, {255,255,255,255}, {1,1}, },	 
	};
	point3D p4 = {200, 200, 0,    0, 0, 0,  
		{ {0, 0}, {255,255,255,255}, {1,-1}, },	 
	};

	update_proj(&p);
	update_proj(&p2);
	update_proj(&p3);
	update_proj(&p4);

	if (!sdl_init(&gWindow, &gRenderer)){
		printf("init failure.\n");
	}else{
		if (!sdl_load(&gRenderer, &gTexture)){
			printf("load failure.\n");
		} else {
			render_scene(&gRenderer, &gTexture, p, p2, p3, p4);

			while (!quit){
				sleep(0.005);

				keystate = SDL_GetKeyboardState(nullptr);
				if (keystate[SDL_SCANCODE_W]){
					p.offz+=0.1; p2.offz+=0.1; p3.offz+=0.1; p4.offz+=0.1;
					update_proj(&p);
					update_proj(&p2);
					update_proj(&p3);
					update_proj(&p4);
					render_scene(&gRenderer, &gTexture, p, p2, p3, p4);
				}
				if (keystate[SDL_SCANCODE_S]){
					p.offz-=0.1; p2.offz-=0.1; p3.offz-=0.1; p4.offz-=0.1;
					update_proj(&p);
					update_proj(&p2);
					update_proj(&p3);
					update_proj(&p4);
					render_scene(&gRenderer, &gTexture, p, p2, p3, p4);
				}
				if (keystate[SDL_SCANCODE_D]){
					p.offx+=0.1; p2.offx+=0.1; p3.offx+=0.1; p4.offx+=0.1;
					update_proj(&p);
					update_proj(&p2);
					update_proj(&p3);
					update_proj(&p4);
					render_scene(&gRenderer, &gTexture, p, p2, p3, p4);
				}
				if (keystate[SDL_SCANCODE_A]){
					p.offx-=0.1; p2.offx-=0.1; p3.offx-=0.1; p4.offx-=0.1;
					update_proj(&p);
					update_proj(&p2);
					update_proj(&p3);
					update_proj(&p4);
					render_scene(&gRenderer, &gTexture, p, p2, p3, p4);
				}
				if (keystate[SDL_SCANCODE_E]){
					p.offy+=0.1; p2.offy+=0.1; p3.offy+=0.1; p4.offy+=0.1;
					update_proj(&p);
					update_proj(&p2);
					update_proj(&p3);
					update_proj(&p4);
					render_scene(&gRenderer, &gTexture, p, p2, p3, p4);
				}
				if (keystate[SDL_SCANCODE_Q]){
					p.offy-=0.1; p2.offy-=0.1; p3.offy-=0.1; p4.offy-=0.1;
					update_proj(&p);
					update_proj(&p2);
					update_proj(&p3);
					update_proj(&p4);
					render_scene(&gRenderer, &gTexture, p, p2, p3, p4);
				}
				if (keystate[SDL_SCANCODE_LEFT]){
					rotate_y(&p);
					rotate_y(&p2);
					rotate_y(&p3);
					rotate_y(&p4);
					update_proj(&p);
					update_proj(&p2);
					update_proj(&p3);
					update_proj(&p4);
					render_scene(&gRenderer, &gTexture, p, p2, p3, p4);
				}
				if (keystate[SDL_SCANCODE_RIGHT]){
					rotate_ny(&p);
					rotate_ny(&p2);
					rotate_ny(&p3);
					rotate_ny(&p4);
					update_proj(&p);
					update_proj(&p2);
					update_proj(&p3);
					update_proj(&p4);
					render_scene(&gRenderer, &gTexture, p, p2, p3, p4);
				}
				if (keystate[SDL_SCANCODE_UP]){
					rotate_x(&p);
					rotate_x(&p2);
					rotate_x(&p3);
					rotate_x(&p4);
					update_proj(&p);
					update_proj(&p2);
					update_proj(&p3);
					update_proj(&p4);
					render_scene(&gRenderer, &gTexture, p, p2, p3, p4);
				}
				if (keystate[SDL_SCANCODE_DOWN]){
					rotate_nx(&p);
					rotate_nx(&p2);
					rotate_nx(&p3);
					rotate_nx(&p4);
					update_proj(&p);
					update_proj(&p2);
					update_proj(&p3);
					update_proj(&p4);
					render_scene(&gRenderer, &gTexture, p, p2, p3, p4);
				}
				//else if (keystate[SDL_SCANCODE_Z]){
				//	if (FOV<1000)
				//		FOV+=0.01;
				//	update_proj(&p);
				//	update_proj(&p2);
				//	update_proj(&p3);
				//	update_proj(&p4);
				//	render_scene(&gRenderer, &gTexture, p, p2, p3, p4);
				//}
				//else if (keystate[SDL_SCANCODE_X]){
				//	if (FOV>0)
				//		FOV-=0.01;
				//	update_proj(&p);
				//	update_proj(&p2);
				//	update_proj(&p3);
				//	update_proj(&p4);
				//	render_scene(&gRenderer, &gTexture, p, p2, p3, p4);
				//}
				while(SDL_PollEvent(&e) > 0){
					if (e.type == SDL_QUIT)
						quit = true;
				}
			}
		}
	}

	sdl_exit(&gWindow, &gRenderer, &gTexture);
}
