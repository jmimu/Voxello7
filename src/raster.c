#include "graph.h"

#include <stdio.h>

#include "dbg.h"
#include "raster.h"
#include <SDL2/SDL_image.h>

struct Raster* lastRaster=0;

struct Raster* raster_load(const char* filename)
{
	struct Raster* raster=(struct Raster*)malloc(sizeof(struct Raster));
	SDL_Surface* surf=0;
	SDL_Surface* surf_conv=0;
	raster->name=(char*)malloc(strlen(filename));
	strcpy(raster->name,filename);

	surf=IMG_Load(filename);
	check(surf!=0,"Impossible to read image file \"%s\"",filename);
	surf_conv=SDL_ConvertSurface(surf,graph.surface->format,0);
	printf("Opened image file \"%s\"\n",filename);
	//printf("File format: %d\n",surf->format->format);
	check(surf_conv->format->format==SDL_PIXELFORMAT_ARGB8888,"Bad format for image file \"%s\", must be ARGB8888",filename);
	printf("Image \"%s\" converted.\n",filename);

	raster->pix=(uint32_t *)malloc(surf->w*surf->h*sizeof(uint32_t));
	memcpy(raster->pix,surf_conv->pixels,surf->w*surf->h*sizeof(uint32_t));
	raster->h=surf->h;
	raster->w=surf->w;


	SDL_FreeSurface(surf);
	SDL_FreeSurface(surf_conv);
	//update raster chain
	raster->previous=lastRaster;
	raster->next=0;
	if (lastRaster) lastRaster->next=raster;
	lastRaster=raster;
	return raster;

error:
	if (surf) SDL_FreeSurface(surf);
	if (surf_conv) SDL_FreeSurface(surf_conv);
	free(raster->name);
	free(raster);
	return 0;
}

void raster_draw(struct Raster* raster, int x, int y,uint16_t z)
{
	long i=0;//raster index
	long j=x+y*graph.render_w;//graph index
	for (int l=0;l<raster->h;l++)
	{
		for (int c=0;c<raster->w;c++)
		{
			if (graph.zbuf[j]>z)
				graph.pixels[j]=raster->pix[i];
			i++;
			j++;
		}
		j+=graph.render_w-raster->w;
	}
}

void raster_draw_zoom(struct Raster* raster, int x, int y, int w, int h)
{
	long i=0;//raster index
	long j=x+y*graph.render_w;//graph index
	for (int l=0;l<raster->h;l++)
	{
		for (int c=0;c<raster->w;c++)
		{
			graph.pixels[j]=raster->pix[i];
			i++;
			j++;
		}
		j+=graph.render_w-raster->w;
	}
}

void raster_unload(struct Raster* raster)
{
	if (raster->next)
		raster->next->previous=raster->previous;
	if (raster->previous)
		raster->previous->next=raster->next;
	if (raster==lastRaster)
		lastRaster=raster->previous;

	free(raster->pix);
	free(raster->name);
}

void raster_unloadall()
{
	struct Raster* next;
	while (lastRaster)
	{
		next=lastRaster->previous;
		raster_unload(lastRaster);
		lastRaster=next;
	}
}
