#include "graph.h"

#include <stdio.h>

#include "dbg.h"
#include "raster.h"
#include <SDL2/SDL_image.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

struct Raster* lastRaster=0;

struct Raster* raster_load(const char* filename)
{
	struct Raster* raster=(struct Raster*)malloc(sizeof(struct Raster));
	SDL_Surface* surf=0;
	SDL_Surface* surf_conv=0;

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
	free(raster);
	return 0;
}

void raster_draw(struct Raster* raster, int x, int y,uint16_t z)
{
	if ((x<0)||(x>graph.render_w-raster->w)||
	    (y<0)||(y>graph.render_h-raster->h))
		return; //out of screen
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

void raster_draw_zoom(struct Raster* raster, int x, int y, uint16_t z, int w, int h)
{
/*	if ((x<0)||(x>graph.render_w-w)||
	    (y<0)||(y>graph.render_h-h))
		return; //out of screen
*/
	int c_start=MAX(0,-x);
	int c_end=MIN(w,graph.render_w-x);
	int l_start=MAX(0,-y);
	int l_end=MIN(h,graph.render_h-y);

	if (x<0)
	{
		if (x<=-w) return;
		c_start=-x;
		x=0;
	}else if (x>graph.render_w-w)
	{
		if (x>=graph.render_w) return;
		c_end=graph.render_w-x;
	}

	if (y<0)
	{
		if (y<=-h) return;
		l_start=-y;
		y=0;
	}else if (y>graph.render_h-h)
	{
		if (y>=graph.render_h) return;
		l_end=graph.render_h-y;
	}


	long i=0;//raster index
	int raster_x,raster_y;
	long j=x+y*graph.render_w;//graph index
	uint32_t color;
	for (int l=l_start;l<l_end;l++)
	{
		raster_y=(l*raster->h)/h;
		i=raster_y*raster->w;
		for (int c=c_start;c<c_end;c++)
		{
			if (graph.zbuf[j]>z)
			{
				raster_x=(c*raster->w)/w;
				color=raster->pix[i+raster_x];
				if ((color&0xFF000000)==0xFF000000)
					graph.pixels[j]=raster->pix[i+raster_x];
			}
			j++;
		}
		j+=graph.render_w-(c_end-c_start);
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
}

void raster_unloadall()
{
	struct Raster* next;
	while (lastRaster)
	{
		next=lastRaster->previous;
		raster_unload(lastRaster);
		free(lastRaster);
		lastRaster=next;
	}
}

//-----------------------------------------------------------
struct Anim* anim_create(float _time_factor)
{
	struct Anim* anim=(struct Anim*)malloc(sizeof(struct Anim));
	anim->time_factor=_time_factor;
	anim->time=0;
	anim->len=0;
	anim->step=0;
	return anim;
}

int anim_add_raster(struct Anim* anim,struct Raster* raster)
{
	check(anim->len<MAXANIMLEN,
		"ERROR, can't have animation with more than %d steps!\n",MAXANIMLEN);
	anim->rasters[anim->len]=raster;
	anim->len++;
	return 1;
error:
	return 0;
}

void anim_frame(struct Anim* anim, float dt)
{
	anim->time+=anim->time_factor*dt;
	anim->step=anim->time/ANIMUNIT;
	if (anim->step>=anim->len)
	{
		anim->time=0;
		anim->step=0;
	}
}

struct Raster* anim_get_raster(struct Anim* anim)
{
	check(anim->len>0,"ERROR, anim is empty!\n");
	return anim->rasters[anim->step];
error:
	return 0;
}

/*void anim_free(struct Anim* anim)
{
	for (uint8_t i=0;i<anim->len;i++)
		raster_unload(anim->rasters[i]);//TODO
	anim->len=0;
	anim->step=0;
}*/

