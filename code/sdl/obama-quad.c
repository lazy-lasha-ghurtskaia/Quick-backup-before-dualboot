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
		*renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED); if (!(*renderer)){ printf("CreateRenderer error: %s\n", SDL_GetError());
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
	*texture = load_texture("obamium.png", renderer);
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

typedef struct triangle3D{
	bool backwards;
	point3D point[3];
} triangle3D;

volatile float FOV = 400;
bool culled(triangle3D* tris)
{
	int i;


	float v0x = tris->point[0].inix + tris->point[0].offx;
	float v0y = tris->point[0].iniy + tris->point[0].offy;
	float v0z = tris->point[0].iniz + tris->point[0].offz + FOV;

	float v1x = tris->point[1].inix + tris->point[1].offx;
	float v1y = tris->point[1].iniy + tris->point[1].offy;
	float v1z = tris->point[1].iniz + tris->point[1].offz + FOV;

	float v2x = tris->point[2].inix + tris->point[2].offx;
	float v2y = tris->point[2].iniy + tris->point[2].offy;
	float v2z = tris->point[2].iniz + tris->point[2].offz + FOV;

	if ((v2z < 0) && (v1z < 0) && (v0z < 0))
		return true;
	else if ((v2z > 5000) && (v1z > 5000) && (v0z > 5000))
		return true;

	// normals of sides
	// 4 directions: left wall, right wall, top wall, bottom wall.
	float h_angle = atan((WINDOW_W/2) / FOV);
	float v_angle = atan((WINDOW_H/2) / FOV);

	float wlx = cos(h_angle);
	float wly = 0;
	float wlz = sin(h_angle);

	float wrx = -cos(h_angle);
	float wry = 0;
	float wrz = sin(h_angle);

	float wtx = 0;
	float wty = -cos(v_angle);
	float wtz = sin(v_angle);

	float wbx = 0;
	float wby = cos(v_angle);
	float wbz = sin(v_angle);

	float p0_wl = (v0x * wlx) + (v0y * wly) + (v0z * wlz);
	float p0_wr = (v0x * wrx) + (v0y * wry) + (v0z * wrz);
	float p0_wt = (v0x * wtx) + (v0y * wty) + (v0z * wtz);
	float p0_wb = (v0x * wbx) + (v0y * wby) + (v0z * wbz);

	float p1_wl = (v1x * wlx) + (v1y * wly) + (v1z * wlz);
	float p1_wr = (v1x * wrx) + (v1y * wry) + (v1z * wrz);
	float p1_wt = (v1x * wtx) + (v1y * wty) + (v1z * wtz);
	float p1_wb = (v1x * wbx) + (v1y * wby) + (v1z * wbz);

	float p2_wl = (v2x * wlx) + (v2y * wly) + (v2z * wlz);
	float p2_wr = (v2x * wrx) + (v2y * wry) + (v2z * wrz);
	float p2_wt = (v2x * wtx) + (v2y * wty) + (v2z * wtz);
	float p2_wb = (v2x * wbx) + (v2y * wby) + (v2z * wbz);
	printf("\nNormal left: (%.02f, %.02f, %.02f)\n", wlx, wly, wlz);
	printf("Normal right: (%.02f, %.02f, %.02f)\n", wrx, wry, wrz);
	printf("Normal top: (%.02f, %.02f, %.02f)\n", wtx, wty, wtz);
	printf("Normal bott: (%.02f, %.02f, %.02f)\n", wbx, wby, wbz);

	//printf("p0: (%.02f, %.02f, %.02f)\n", p0_wl, p1_wl, p2_wl);
	printf("position p0: (%.02f, %.02f, %.02f)\n", v0x, v0y, v0z);


	if ((p0_wl<0) && (p1_wl<0) && (p2_wl<0)){
		printf("culled left\n");
		return true;
	}
	if ((p0_wr<0) && (p1_wr<0) && (p2_wr<0)){
		printf("culled right\n");
		return true;
	}
	if ((p0_wt<0) && (p1_wt<0) && (p2_wt<0)){
		printf("culled top\n");
		return true;
	}
	if ((p0_wb<0) && (p1_wb<0) && (p2_wb<0)){
		printf("culled bottom\n");
		return true;
	}

	//printf("p0: (%.02f, %.02f, %.02f)\n", v0x, v0y, v0z);
	//printf("DotProduct: %.02f\n", along_p0);

	return false;
}

void render_scene(SDL_Renderer** renderer, SDL_Texture** texture, triangle3D** tris, int numtris)
{
	SDL_Vertex verts[3];
	int p0x;
	int p0y;
	int p1x;
	int p1y;
	int p2x;
	int p2y;

	SDL_SetRenderDrawColor(*renderer, 0, 0, 0, 0xff);
	SDL_RenderClear(*renderer);

	for (int i = 0; i< numtris; i++){
		p0x = (*tris)[i].point[0].pt.position.x;
		p0y = (*tris)[i].point[0].pt.position.y;

		p1x = (*tris)[i].point[1].pt.position.x;
		p1y = (*tris)[i].point[1].pt.position.y;

		p2x = (*tris)[i].point[2].pt.position.x;
		p2y = (*tris)[i].point[2].pt.position.y;

		if ( (*tris)[i].backwards )
			break;
		if ( culled( &(*tris)[i] ) )
			break;

		verts[0] = (*tris)[i].point[0].pt;
		verts[1] = (*tris)[i].point[1].pt;
		verts[2] = (*tris)[i].point[2].pt;
		SDL_RenderGeometry(*renderer, *texture, verts, 3, NULL, 0);
	}

	SDL_SetRenderDrawColor(*renderer, 0xff, 0xff, 0, 0xff);
	SDL_RenderDrawLine(*renderer, WINDOW_W/2, 0, WINDOW_W/2, WINDOW_H);
	SDL_SetRenderDrawColor(*renderer, 0, 0xff, 0xff, 0xff);
	SDL_RenderDrawLine(*renderer, 0, WINDOW_H/2, WINDOW_W, WINDOW_H/2);

	SDL_RenderPresent(*renderer);
}

#define THETA  0.0008726646
void update_proj(triangle3D** tris, int numtris)
{
	for (int i = 0; i< numtris; i++){
		(*tris)[i].point[0].pt.position.x = ( (((*tris)[i].point[0].offx + (*tris)[i].point[0].inix) * FOV) / (((*tris)[i].point[0].offz + (*tris)[i].point[0].iniz) + FOV) ) + (WINDOW_W/2);
		(*tris)[i].point[0].pt.position.y = (WINDOW_H/2) - ( (((*tris)[i].point[0].offy + (*tris)[i].point[0].iniy)*FOV) / (((*tris)[i].point[0].offz + (*tris)[i].point[0].iniz)+FOV) );
		
		(*tris)[i].point[1].pt.position.x = ( (((*tris)[i].point[1].offx + (*tris)[i].point[1].inix) * FOV) / (((*tris)[i].point[1].offz + (*tris)[i].point[1].iniz) + FOV) ) + (WINDOW_W/2);
		(*tris)[i].point[1].pt.position.y = (WINDOW_H/2) - ( (((*tris)[i].point[1].offy + (*tris)[i].point[1].iniy)*FOV) / (((*tris)[i].point[1].offz + (*tris)[i].point[1].iniz)+FOV) );

		(*tris)[i].point[2].pt.position.x = ( (((*tris)[i].point[2].offx + (*tris)[i].point[2].inix) * FOV) / (((*tris)[i].point[2].offz + (*tris)[i].point[2].iniz) + FOV) ) + (WINDOW_W/2);
		(*tris)[i].point[2].pt.position.y = (WINDOW_H/2) - ( (((*tris)[i].point[2].offy + (*tris)[i].point[2].iniy)*FOV) / (((*tris)[i].point[2].offz + (*tris)[i].point[2].iniz)+FOV) );
	}
}

void calc_shading(triangle3D* tris)
{
	float v0x = tris->point[0].inix + tris->point[0].offx;
	float v0y = tris->point[0].iniy + tris->point[0].offy;
	float v0z = tris->point[0].iniz + tris->point[0].offz;

	float v1x = tris->point[1].inix + tris->point[1].offx;
	float v1y = tris->point[1].iniy + tris->point[1].offy;
	float v1z = tris->point[1].iniz + tris->point[1].offz;

	float v2x = tris->point[2].inix + tris->point[2].offx;
	float v2y = tris->point[2].iniy + tris->point[2].offy;
	float v2z = tris->point[2].iniz + tris->point[2].offz;

	float cx = (v0x + v1x + v2x)/3.0f;
	float cy = (v0y + v1y + v2y)/3.0f;
	float cz = (v0z + v1z + v2z)/3.0f;


	float ax = v1x - v0x;
	float ay = v1y - v0y;
	float az = v1z - v0z;

	float bx = v2x - v0x;
	float by = v2y - v0y;
	float bz = v2z - v0z;

	float nx = ay*bz - az*by;
	float ny = az*bx - ax*bz;
	float nz = ax*by - ay*bx; 

	float len2 = sqrtf((nx*nx) + (ny*ny) + (nz*nz));
	//if (len2 < FLT_EPSILON) len2 = 1.0f;
	nx /= len2; ny /= len2; nz /= len2;


	float camx = 0.0f;
	float camy = 0.0f;
	float camz = -FOV;

	float vx = camx - cx;
	float vy = camy - cy;
	float vz = camz - cz;

	float vlen = sqrtf(vx*vx + vy*vy + vz*vz);
	//if (vlen < FLT_EPSILON) vlen = 1.0f;
	vx /= vlen;
	vy /= vlen;
	vz /= vlen;

	float dot = (nx*vx) + (ny*vy) + (nz*vz);
	tris->backwards = dot < 0 ? false : true;
	//printf("Normal: (%.3f, %.3f, %.3f)\n", nx, ny, nz);
    //printf("Centroid: (%.3f, %.3f, %.3f)\n", cx, cy, cz);
    //printf("Camera vec: (%.3f, %.3f, %.3f)\n", vx, vy, vz);
    //printf("Dot product (N·V): %.3f\n", dot);

	float oldRange = -1;
	float newRange = 255;
	float finalCol = (((dot) * newRange) / oldRange);

	tris->point[0].pt.color.r = finalCol;
	tris->point[0].pt.color.g = finalCol;
	tris->point[0].pt.color.b = finalCol;
	tris->point[1].pt.color.r = finalCol;
	tris->point[1].pt.color.g = finalCol;
	tris->point[1].pt.color.b = finalCol;
	tris->point[2].pt.color.r = finalCol;
	tris->point[2].pt.color.g = finalCol;
	tris->point[2].pt.color.b = finalCol;
}

void rotate_y(triangle3D** tris, int numtris)
{
	int i;
	float finx;
	float finz;

	float avex;
	float avez;

	// get average for all points
	for (i = 0; i< numtris; i++){
		avex += (*tris)[i].point[0].inix;
		avez += (*tris)[i].point[0].iniz;

		avex += (*tris)[i].point[1].inix;
		avez += (*tris)[i].point[1].iniz;

		avex += (*tris)[i].point[2].inix;
		avez += (*tris)[i].point[2].iniz;
	}
	avex /= numtris*3;
	avez /= numtris*3;

	// subtract average from points
	// rotate all triangles
	// add average from points
	// do offseted rotation like with camera case

	for (i = 0; i< numtris; i++){
		(*tris)[i].point[0].inix -= avex;
		(*tris)[i].point[0].iniz -= avez;

		(*tris)[i].point[1].inix -= avex;
		(*tris)[i].point[1].iniz -= avez;

		(*tris)[i].point[2].inix -= avex;
		(*tris)[i].point[2].iniz -= avez;

		finx = ((*tris)[i].point[0].inix * cos(THETA)) + ((*tris)[i].point[0].iniz * sin(THETA));
		finz = ((*tris)[i].point[0].inix * -(sin(THETA))) + ((*tris)[i].point[0].iniz * cos(THETA));
		(*tris)[i].point[0].inix = finx + avex;
		(*tris)[i].point[0].iniz = finz + avez;

		finx = ((*tris)[i].point[1].inix * cos(THETA)) + ((*tris)[i].point[1].iniz * sin(THETA));
		finz = ((*tris)[i].point[1].inix * -(sin(THETA))) + ((*tris)[i].point[1].iniz * cos(THETA));
		(*tris)[i].point[1].inix = finx + avex;
		(*tris)[i].point[1].iniz = finz + avez;

		finx = ((*tris)[i].point[2].inix * cos(THETA)) + ((*tris)[i].point[2].iniz * sin(THETA));
		finz = ((*tris)[i].point[2].inix * -(sin(THETA))) + ((*tris)[i].point[2].iniz * cos(THETA));
		(*tris)[i].point[2].inix = finx + avex;
		(*tris)[i].point[2].iniz = finz + avez;

		calc_shading(&((*tris)[i]));
	}
}
void rotate_ny(triangle3D** tris, int numtris)
{
	int i;
	float finx;
	float finz;

	float avex;
	float avez;
	
	for (i = 0; i< numtris; i++){
		avex += (*tris)[i].point[0].inix;
		avez += (*tris)[i].point[0].iniz;

		avex += (*tris)[i].point[1].inix;
		avez += (*tris)[i].point[1].iniz;

		avex += (*tris)[i].point[2].inix;
		avez += (*tris)[i].point[2].iniz;
	}
	avex /= numtris*3;
	avez /= numtris*3;

	for (i = 0; i< numtris; i++){
		(*tris)[i].point[0].inix -= avex;
		(*tris)[i].point[0].iniz -= avez;

		(*tris)[i].point[1].inix -= avex;
		(*tris)[i].point[1].iniz -= avez;

		(*tris)[i].point[2].inix -= avex;
		(*tris)[i].point[2].iniz -= avez;

		finx = ((*tris)[i].point[0].inix * cos(-THETA)) + ((*tris)[i].point[0].iniz * sin(-THETA));
		finz = ((*tris)[i].point[0].inix * -(sin(-THETA))) + ((*tris)[i].point[0].iniz * cos(-THETA));
		(*tris)[i].point[0].inix = finx + avex;
		(*tris)[i].point[0].iniz = finz + avez;

		finx = ((*tris)[i].point[1].inix * cos(-THETA)) + ((*tris)[i].point[1].iniz * sin(-THETA));
		finz = ((*tris)[i].point[1].inix * -(sin(-THETA))) + ((*tris)[i].point[1].iniz * cos(-THETA));
		(*tris)[i].point[1].inix = finx + avex;
		(*tris)[i].point[1].iniz = finz + avez;

		finx = ((*tris)[i].point[2].inix * cos(-THETA)) + ((*tris)[i].point[2].iniz * sin(-THETA));
		finz = ((*tris)[i].point[2].inix * -(sin(-THETA))) + ((*tris)[i].point[2].iniz * cos(-THETA));
		(*tris)[i].point[2].inix = finx + avex;
		(*tris)[i].point[2].iniz = finz + avez;

		calc_shading(&((*tris)[i]));
	}
}

void rotate_window_y(triangle3D** tris, int numtris)
{
	float finx;
	float finz;
	float angle = THETA*2;
	for (int i = 0; i< numtris; i++){
		(*tris)[i].point[0].iniz += FOV;
		finx = (((*tris)[i].point[0].inix + (*tris)[i].point[0].offx) * cos(angle)) + (((*tris)[i].point[0].iniz + (*tris)[i].point[0].offz) * sin(angle));
		finz = (((*tris)[i].point[0].inix + (*tris)[i].point[0].offx) * -(sin(angle))) + (((*tris)[i].point[0].iniz + (*tris)[i].point[0].offz) * cos(angle));
		(*tris)[i].point[0].inix = finx - (*tris)[i].point[0].offx;
		(*tris)[i].point[0].iniz = finz-FOV-(*tris)[i].point[0].offz;

		(*tris)[i].point[1].iniz += FOV;
		finx = (((*tris)[i].point[1].inix + (*tris)[i].point[1].offx) * cos(angle)) + (((*tris)[i].point[1].iniz + (*tris)[i].point[1].offz) * sin(angle));
		finz = (((*tris)[i].point[1].inix + (*tris)[i].point[1].offx) * -(sin(angle))) + (((*tris)[i].point[1].iniz + (*tris)[i].point[1].offz) * cos(angle));
		(*tris)[i].point[1].inix = finx - (*tris)[i].point[1].offx;
		(*tris)[i].point[1].iniz = finz-FOV-(*tris)[i].point[1].offz;

		(*tris)[i].point[2].iniz += FOV;
		finx = (((*tris)[i].point[2].inix + (*tris)[i].point[2].offx) * cos(angle)) + (((*tris)[i].point[2].iniz + (*tris)[i].point[2].offz) * sin(angle));
		finz = (((*tris)[i].point[2].inix + (*tris)[i].point[2].offx) * -(sin(angle))) + (((*tris)[i].point[2].iniz + (*tris)[i].point[2].offz) * cos(angle));
		(*tris)[i].point[2].inix = finx - (*tris)[i].point[2].offx;
		(*tris)[i].point[2].iniz = finz-FOV-(*tris)[i].point[2].offz;
		calc_shading(&((*tris)[i]));
	}
}
void rotate_window_ny(triangle3D** tris, int numtris)
{
	float finx;
	float finz;
	float angle = -THETA*2;

	for (int i = 0; i< numtris; i++){
		(*tris)[i].point[0].iniz += FOV;
		finx = (((*tris)[i].point[0].inix + (*tris)[i].point[0].offx) * cos(angle)) + (((*tris)[i].point[0].iniz + (*tris)[i].point[0].offz) * sin(angle));
		finz = (((*tris)[i].point[0].inix + (*tris)[i].point[0].offx) * -(sin(angle))) + (((*tris)[i].point[0].iniz + (*tris)[i].point[0].offz) * cos(angle));
		(*tris)[i].point[0].inix = finx - (*tris)[i].point[0].offx;
		(*tris)[i].point[0].iniz = finz-FOV-(*tris)[i].point[0].offz;

		(*tris)[i].point[1].iniz += FOV;
		finx = (((*tris)[i].point[1].inix + (*tris)[i].point[1].offx) * cos(angle)) + (((*tris)[i].point[1].iniz + (*tris)[i].point[1].offz) * sin(angle));
		finz = (((*tris)[i].point[1].inix + (*tris)[i].point[1].offx) * -(sin(angle))) + (((*tris)[i].point[1].iniz + (*tris)[i].point[1].offz) * cos(angle));
		(*tris)[i].point[1].inix = finx - (*tris)[i].point[1].offx;
		(*tris)[i].point[1].iniz = finz-FOV-(*tris)[i].point[1].offz;

		(*tris)[i].point[2].iniz += FOV;
		finx = (((*tris)[i].point[2].inix + (*tris)[i].point[2].offx) * cos(angle)) + (((*tris)[i].point[2].iniz + (*tris)[i].point[2].offz) * sin(angle));
		finz = (((*tris)[i].point[2].inix + (*tris)[i].point[2].offx) * -(sin(angle))) + (((*tris)[i].point[2].iniz + (*tris)[i].point[2].offz) * cos(angle));
		(*tris)[i].point[2].inix = finx - (*tris)[i].point[2].offx;
		(*tris)[i].point[2].iniz = finz-FOV-(*tris)[i].point[2].offz;
		calc_shading(&((*tris)[i]));
	}
}

void rotate_x(triangle3D** tris, int numtris)
{
	int i;
	float finy;
	float finz;

	float avey;
	float avez;
	
	for (i = 0; i< numtris; i++){
		avey += (*tris)[i].point[0].iniy;
		avez += (*tris)[i].point[0].iniz;

		avey += (*tris)[i].point[1].iniy;
		avez += (*tris)[i].point[1].iniz;

		avey += (*tris)[i].point[2].iniy;
		avez += (*tris)[i].point[2].iniz;
	}
	avey /= numtris*3;
	avez /= numtris*3;

	for (i = 0; i< numtris; i++){
		(*tris)[i].point[0].iniy -= avey;
		(*tris)[i].point[0].iniz -= avez;

		(*tris)[i].point[1].iniy -= avey;
		(*tris)[i].point[1].iniz -= avez;

		(*tris)[i].point[2].iniy -= avey;
		(*tris)[i].point[2].iniz -= avez;

		finy = ((*tris)[i].point[0].iniy * cos(THETA)) + ((*tris)[i].point[0].iniz * sin(THETA));
		finz = ((*tris)[i].point[0].iniy * -(sin(THETA))) + ((*tris)[i].point[0].iniz * cos(THETA));
		(*tris)[i].point[0].iniy = finy + avey;
		(*tris)[i].point[0].iniz = finz + avez;

		finy = ((*tris)[i].point[1].iniy * cos(THETA)) + ((*tris)[i].point[1].iniz * sin(THETA));
		finz = ((*tris)[i].point[1].iniy * -(sin(THETA))) + ((*tris)[i].point[1].iniz * cos(THETA));
		(*tris)[i].point[1].iniy = finy + avey;
		(*tris)[i].point[1].iniz = finz + avez;

		finy = ((*tris)[i].point[2].iniy * cos(THETA)) + ((*tris)[i].point[2].iniz * sin(THETA));
		finz = ((*tris)[i].point[2].iniy * -(sin(THETA))) + ((*tris)[i].point[2].iniz * cos(THETA));
		(*tris)[i].point[2].iniy = finy + avey;
		(*tris)[i].point[2].iniz = finz + avez;
		calc_shading(&((*tris)[i]));
	}
}
void rotate_nx(triangle3D** tris, int numtris)
{
	int i;
	float finy;
	float finz;

	float avey;
	float avez;
	
	for (i = 0; i< numtris; i++){
		avey += (*tris)[i].point[0].iniy;
		avez += (*tris)[i].point[0].iniz;

		avey += (*tris)[i].point[1].iniy;
		avez += (*tris)[i].point[1].iniz;

		avey += (*tris)[i].point[2].iniy;
		avez += (*tris)[i].point[2].iniz;
	}
	avey /= numtris*3;
	avez /= numtris*3;

	for (i = 0; i< numtris; i++){
		(*tris)[i].point[0].iniy -= avey;
		(*tris)[i].point[0].iniz -= avez;

		(*tris)[i].point[1].iniy -= avey;
		(*tris)[i].point[1].iniz -= avez;

		(*tris)[i].point[2].iniy -= avey;
		(*tris)[i].point[2].iniz -= avez;

		finy = ((*tris)[i].point[0].iniy * cos(-THETA)) + ((*tris)[i].point[0].iniz * sin(-THETA));
		finz = ((*tris)[i].point[0].iniy * -(sin(-THETA))) + ((*tris)[i].point[0].iniz * cos(-THETA));
		(*tris)[i].point[0].iniy = finy + avey;
		(*tris)[i].point[0].iniz = finz + avez;

		finy = ((*tris)[i].point[1].iniy * cos(-THETA)) + ((*tris)[i].point[1].iniz * sin(-THETA));
		finz = ((*tris)[i].point[1].iniy * -(sin(-THETA))) + ((*tris)[i].point[1].iniz * cos(-THETA));
		(*tris)[i].point[1].iniy = finy + avey;
		(*tris)[i].point[1].iniz = finz + avez;

		finy = ((*tris)[i].point[2].iniy * cos(-THETA)) + ((*tris)[i].point[2].iniz * sin(-THETA));
		finz = ((*tris)[i].point[2].iniy * -(sin(-THETA))) + ((*tris)[i].point[2].iniz * cos(-THETA));
		(*tris)[i].point[2].iniy = finy + avey;
		(*tris)[i].point[2].iniz = finz + avez;
		calc_shading(&((*tris)[i]));
	}
}

void add_comp(triangle3D** tris, int numtris, float x, float y, float z)
{
	for (int i = 0; i< numtris; i++){
		if (x != 0){
			(*tris)[i].point[0].offx += x; 
			(*tris)[i].point[1].offx += x; 
			(*tris)[i].point[2].offx += x; 
		}
		if (y != 0) {
			(*tris)[i].point[0].offy += y; 
			(*tris)[i].point[1].offy += y; 
			(*tris)[i].point[2].offy += y; 
		} 
		if (z != 0) {
			(*tris)[i].point[0].offz += z; 
			(*tris)[i].point[1].offz += z; 
			(*tris)[i].point[2].offz += z; 
		}
		if (i == 2)
			printf("\n\nthird triangle here:\n");
		calc_shading(&((*tris)[i]));
	}

}

// TODO:
// [X] 3d fps camera (double axis rotation problem)
// [X] uniform shading 
// [X] backface culling
// [X] frustum culling
// [ ] triangle groups
// [ ] obj file importing
// [ ] textures

int main(int argc, char **argv)
{
	SDL_Window* gWindow = NULL;
	SDL_Renderer* gRenderer = NULL;
	SDL_Texture* gTexture = NULL;

	SDL_Event e;
	bool quit = false;
	const Uint8* keystate;


	//triangle3D *tris = malloc(2*sizeof(triangle3D));

	//tris[0].backwards = false;
	//tris[0].point[0] = (point3D){ -200, 200, 0,    0, 0, 0,  
	//	{ {0, 0}, {255,255,255,255}, {1,0}, },	 
	//};
	//tris[0].point[1] = (point3D){ -200, -200, 0,    0, 0, 0,  
	//	{ {0, 0}, {255,255,255,255}, {1,1}, },	 
	//};
	//tris[0].point[2] = (point3D){ 200, -200, 0,    0, 0, 0,  
	//	{ {0, 0}, {255,255,255,255}, {0,1}, },	 
	//};

	//tris[1].backwards = false;
	//tris[1].point[0] = (point3D){ -200, 200, 0,    0, 0, 0,  
	//	{ {0, 0}, {255,255,255,255}, {1,0}, },	 
	//};
	//tris[1].point[1] = (point3D){ 200, -200, 0,    0, 0, 0,  
	//	{ {0, 0}, {255,255,255,255}, {0,1}, },	 
	//};
	//tris[1].point[2] = (point3D){ 200, 200, 0,    0, 0, 0,  
	//	{ {0, 0}, {255,255,255,255}, {0,0}, },	 
	//};

	int fa = 200;
	triangle3D *tris = malloc(6*sizeof(triangle3D));

	tris[0].backwards = false;
	tris[0].point[0] = (point3D){ 0, fa*1, 0,    0, 0, 200,  
		{ {0, 0}, {255,255,255,255}, {0.5,-0.2}, },	 
	};
	tris[0].point[1] = (point3D){ fa*-1, 0, fa*-1,    0, -100, 200,  
		{ {0, 0}, {255,255,255,255}, {1,1}, },	 
	};
	tris[0].point[2] = (point3D){ fa*1, 0, fa*-1,    0, -100, 200,  
		{ {0, 0}, {255,255,255,255}, {0,1}, },	 
	};

	tris[1].backwards = false;
	tris[1].point[0] = (point3D){ 0, fa*1, 0,    0, 0, 200,  
		{ {0, 0}, {255,255,255,255}, {0.5,-0.2}, },	 
	};
	tris[1].point[1] = (point3D){ fa*1, 0, fa*-1,    0, -100, 200,  
		{ {0, 0}, {255,255,255,255}, {1,1}, },	 
	};
	tris[1].point[2] = (point3D){ fa*1, 0, fa*1,    0, -100, 200,  
		{ {0, 0}, {255,255,255,255}, {0,1}, },	 
	};

	tris[2].backwards = false;
	tris[2].point[1] = (point3D){ 0, fa*1, 0,    0, 0, 200,  
		{ {0, 0}, {255,255,255,255}, {1,0}, },	 
	};
	tris[2].point[2] = (point3D){ fa*1, 0, fa*1,    0, -100, 200,  
		{ {0, 0}, {255,255,255,255}, {0,1}, },	 
	};
	tris[2].point[0] = (point3D){ fa*-1, 0, fa*1,    0, -100, 200,  
		{ {0, 0}, {255,255,255,255}, {0,0}, },	 
	};

	tris[3].backwards = false;
	tris[3].point[1] = (point3D){ 0, fa*1, 0,    0, 0, 200,  
		{ {0, 0}, {255,255,255,255}, {1,0}, },	 
	};
	tris[3].point[2] = (point3D){ fa*-1, 0, fa*1,    0, -100, 200,  
		{ {0, 0}, {255,255,255,255}, {0,1}, },	 
	};
	tris[3].point[0] = (point3D){ fa*-1, 0, fa*-1,    0, -100, 200,  
		{ {0, 0}, {255,255,255,255}, {0,0}, },	 
	};
	// array of points and an array that would group them

	int numtris = 4;

	update_proj(&tris, numtris);

	if (!sdl_init(&gWindow, &gRenderer)){
		printf("init failure.\n");
	}else{
		if (!sdl_load(&gRenderer, &gTexture)){
			printf("load failure.\n");
		} else {
			render_scene(&gRenderer, &gTexture, &tris, numtris);

			while (!quit){
				sleep(0.1);

				keystate = SDL_GetKeyboardState(nullptr);
				if (keystate[SDL_SCANCODE_W]){
					add_comp(&tris, numtris,  0, 0, -0.3);
					update_proj(&tris, numtris);
					render_scene(&gRenderer, &gTexture, &tris, numtris);
				}
				if (keystate[SDL_SCANCODE_S]){
					add_comp(&tris, numtris,   0, 0, 0.3);
					update_proj(&tris, numtris);
					render_scene(&gRenderer, &gTexture, &tris, numtris);
				}
				if (keystate[SDL_SCANCODE_D]){
					add_comp(&tris, numtris,  -0.3, 0, 0);
					update_proj(&tris, numtris);
					render_scene(&gRenderer, &gTexture, &tris, numtris);
				}
				if (keystate[SDL_SCANCODE_A]){
					add_comp(&tris, numtris,  0.3, 0, 0);
					update_proj(&tris, numtris);
					render_scene(&gRenderer, &gTexture, &tris, numtris);
				}
				if (keystate[SDL_SCANCODE_E]){
					add_comp(&tris, numtris,  0, 0.3, 0);
					update_proj(&tris, numtris);
					render_scene(&gRenderer, &gTexture, &tris, numtris);
				}
				if (keystate[SDL_SCANCODE_Q]){
					add_comp(&tris, numtris,  0, -0.3, 0);
					update_proj(&tris, numtris);
					render_scene(&gRenderer, &gTexture, &tris, numtris);
				}

				if (keystate[SDL_SCANCODE_LEFT]){
					rotate_y(&tris, numtris);
					update_proj(&tris, numtris);
					render_scene(&gRenderer, &gTexture, &tris, numtris);
				}
				if (keystate[SDL_SCANCODE_RIGHT]){
					rotate_ny(&tris, numtris);
					update_proj(&tris, numtris);
					render_scene(&gRenderer, &gTexture, &tris, numtris);
				}
				if (keystate[SDL_SCANCODE_UP]){
					rotate_nx(&tris, numtris);
					update_proj(&tris, numtris);
					render_scene(&gRenderer, &gTexture, &tris, numtris);
				}
				if (keystate[SDL_SCANCODE_DOWN]){
					rotate_x(&tris, numtris);
					update_proj(&tris, numtris);
					render_scene(&gRenderer, &gTexture, &tris, numtris);
				}

				if (keystate[SDL_SCANCODE_J]){
					rotate_window_y(&tris, numtris);
					update_proj(&tris, numtris);
					render_scene(&gRenderer, &gTexture, &tris, numtris);
				}
				if (keystate[SDL_SCANCODE_L]){
					rotate_window_ny(&tris, numtris);
					update_proj(&tris, numtris);
					render_scene(&gRenderer, &gTexture, &tris, numtris);
				}
				
				else if (keystate[SDL_SCANCODE_Z]){
					if (FOV<1200)
						FOV+=0.1;
					update_proj(&tris, numtris);
					render_scene(&gRenderer, &gTexture, &tris, numtris);
				}
				else if (keystate[SDL_SCANCODE_X]){
					if (FOV>100)
						FOV-=0.1;
					update_proj(&tris, numtris);
					render_scene(&gRenderer, &gTexture, &tris, numtris);
				}
				while(SDL_PollEvent(&e) > 0){
					if (e.type == SDL_QUIT)
						quit = true;
				}
			}
		}
	}

	free(tris);
	sdl_exit(&gWindow, &gRenderer, &gTexture);
}
