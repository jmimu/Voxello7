#include "sky.h"

#include <stdio.h>


struct Sky * createSky()
{
    struct Sky * sky = malloc(sizeof(struct Sky));
    sky->sunSite = -1;
    sky->sunAzimuth =-0.5;
    upSunVect(sky);
    return sky;
}

void upSunVect(struct Sky * sky)
{
    sky->sunVect[0] = sin(sky->sunSite)*sin(sky->sunAzimuth);
    sky->sunVect[1] = sin(sky->sunSite)*cos(sky->sunAzimuth);
    sky->sunVect[2] = cos(sky->sunSite);
    //printf("sun: %f %f => [%f %f %f]\n",sky->sunSite,sky->sunAzimuth,
    //       sky->sunVect[0], sky->sunVect[1], sky->sunVect[2]);
}
