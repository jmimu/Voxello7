#include "voxtif.h"
#include "../dbg.h"
#include <tiffio.h>

/*void add_vox(struct VoxWorld * world, int x, int y, int z, int v)
{
    voxworld_expand_col(world,x,y);
    world->curr_exp_col[z]=v+30;
    voxworld_compr_col(world);
    voxworld_write_compr_col(world,x,y);
}*/

struct VoxWorld * VoxWorld_create_from_tif(const char *pathDTM, const char *pathRGB, double pix_size)
{
    TIFF* dtm = TIFFOpen(pathDTM, "r");
    TIFF* rgb = NULL;
    uint32 w, h, w_rgb, h_rgb;
	size_t npixels;
	float* dtm_buffer=NULL;
    uint8* rgb_buffer=NULL;
    float* dtm_data=NULL;
    uint8* rgb_data=NULL;
    
    float* dtm_p=NULL; //pointer inside dtm_data
    uint8* rgb_p=NULL;
    
    struct VoxWorld * world = NULL;
    
    
    uint16 tiffBps;
    uint16 tiffSpp;
	TIFFGetField(dtm, TIFFTAG_IMAGEWIDTH, &w);
	TIFFGetField(dtm, TIFFTAG_IMAGELENGTH, &h);
	TIFFGetField(dtm, TIFFTAG_BITSPERSAMPLE, &tiffBps);
    TIFFGetField(dtm, TIFFTAG_SAMPLESPERPIXEL, &tiffSpp);
	npixels = w * h;
    printf("dtm tiff file %s size: %d %d\n", pathDTM, w, h);
    printf("dtm tiff file tiffBps=%d tiffSpp=%d\n", tiffBps, tiffSpp);
    if ((tiffBps!=32) || (tiffSpp!=1))
    {
        printf("Error: dtm tiff file is not float 1spp!\n");
	    goto VoxWorld_create_from_tif_end;
    }
    
    if (pathRGB)
    {
        rgb = TIFFOpen(pathRGB, "r");
      	TIFFGetField(dtm, TIFFTAG_IMAGEWIDTH, &w_rgb);
	    TIFFGetField(dtm, TIFFTAG_IMAGELENGTH, &h_rgb);
	    TIFFGetField(dtm, TIFFTAG_BITSPERSAMPLE, &tiffBps);
        TIFFGetField(dtm, TIFFTAG_SAMPLESPERPIXEL, &tiffSpp);
        printf("rgb tiff file %s size: %d %d\n", pathDTM, w_rgb, h_rgb);
        printf("rgb tiff file tiffBps=%d tiffSpp=%d\n", tiffBps, tiffSpp);
        if ((w!=w_rgb) || (h!=h_rgb))
        {
            printf("Error: dtm and rgb size are not the same!\n");
	        goto VoxWorld_create_from_tif_end;
        }
        if ((tiffBps!=32) || (tiffSpp!=1))
        {
            printf("Error: rgb tiff file must be 32bit!\n");
	        goto VoxWorld_create_from_tif_end;
        }
    }

    dtm_data = dtm_p = (float*) malloc(npixels*sizeof(float));
    dtm_buffer = (float*)_TIFFmalloc(TIFFScanlineSize(dtm));
    if (rgb)
    {
        rgb_data = rgb_p = (uint8*) malloc(npixels*3*sizeof(uint8));
        rgb_buffer = (uint8*)_TIFFmalloc(TIFFScanlineSize(rgb));
        printf("TIFFScanlineSize(rgb) = %d\n",TIFFScanlineSize(rgb));
    }
    for (uint32 l=0; l<h; ++l)
    {
        TIFFReadScanline(dtm,dtm_buffer,l,0);
        memcpy(dtm_p, dtm_buffer, w*sizeof(float));
        dtm_p += w;
        if (rgb)
        {
            TIFFReadScanline(rgb,rgb_buffer,l,0);
            memcpy(rgb_p, rgb_buffer, w*3*sizeof(uint8));
            rgb_p += 3*w;
        }
    }
    
    float z_min = 100000;
    float z_max = -100000;
    float z;
    dtm_p = dtm_data;
    for (uint32 i=0;i<npixels;++i)
    {
        z = *dtm_p;
        if (z>z_max) z_max=z;
        if (z<z_min) z_min=z;
        ++dtm_p;
    }
    printf("z amplitude: %f %f\n", z_min, z_max);
    
    int nb_z = ((z_max-z_min)/pix_size + 1)*2; //*2 to be albe to fly above
    
    //pictures read
    world = voxworld_create(w,h,nb_z);

    int x,y,col;
    printf("Filling world...\n");

    //prepare compressed col, always only 2 rle blocks
    world->curr_compr_col_size = 2;
    world->curr_compr_col[1].v = EMPTY;
    world->curr_col_full_start = 0;
    dtm_p = dtm_data;
    rgb_p = rgb_data;
    col = 100; //if no rgb
    for (x=0;x<world->szX;x++)
        for (y=0;y<world->szY;y++)
        {
            z = *dtm_p;
            if (rgb)
            {
                col = color_24to15((* ((uint32_t*)rgb_p))<<8);
                rgb_p+=3;
            }
            world->curr_compr_col[0].v = col;
            world->curr_compr_col[0].n = ((z-z_min)/pix_size+1);
            world->curr_compr_col[1].n = nb_z - world->curr_compr_col[0].n;
            world->curr_col_full_end = world->curr_compr_col[0].n;
            voxworld_write_compr_col(world,x,y);
            ++dtm_p;
        }
    
    
VoxWorld_create_from_tif_end:
    printf("cleaning...\n");
    if (dtm_buffer) _TIFFfree(dtm_buffer);
    TIFFClose(dtm);
    if (dtm_data) free(dtm_data);
    if (rgb_data) free(rgb_data);
    if (rgb_buffer) _TIFFfree(rgb_buffer);
    if (rgb) TIFFClose(rgb);
    return world;
}

