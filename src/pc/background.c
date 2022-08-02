#include "background.h"
#include "graph.h"
#include "trigo.h"
#include <SDL2/SDL_image.h>


struct Background * background_create(char* filename)
{
	struct Background * bg= (struct Background *) malloc(sizeof(struct Background));
    printf("Read background from %s\n",filename);
    bg->background = IMG_Load(filename);
	bg->background=SDL_ConvertSurface(bg->background,graph.surface->format,0);
	//bg->background = SDL_CreateTextureFromSurface(graph.renderer, backsurf);
	/*SDL_FreeSurface(backsurf);
	SDL_FreeSurface(backsurf_conv);

	SDL_QueryTexture(bg->background, NULL, NULL,&bg->w,&bg->h);
	bg->w/=2;*/
    bg->h = bg->background->h;
    bg->w = bg->background->w;

	return bg;
}

void background_draw(struct Background * bg, double ang_l, double ang_r)
{
	static SDL_Rect SrcR;
	//ang_l*=graph.render2ScreenFactor;
	//ang_r*=graph.render2ScreenFactor;
	if (ang_l<0) {ang_l+=2*PI;ang_r+=2*PI;}
	if (ang_r>4*PI) {ang_l-=2*PI;ang_r-=2*PI;}
	SrcR.x = ang_l*bg->w/(2*PI);
	SrcR.y = 0;
	SrcR.w = (ang_r-ang_l)*bg->w/(2*PI);
	SrcR.h = bg->h;

    //SDL_RenderCopy(graph.renderer, bg->background, &SrcR, NULL);
    SDL_BlitSurface(&bg->background, &SrcR, &graph.surface, NULL);

}

void background_delete(struct Background * bg)
{
	SDL_FreeSurface(bg->background);
}
