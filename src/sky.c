#include "sky.h"

#include <stdio.h>
#include <stdlib.h>

struct Sky * createSky()
{
    struct Sky * sky = malloc(sizeof(struct Sky));
    sky->sunSite = -0.5;
    sky->sunAzimuth = 0;
    upSunVect(sky);
    return sky;
}

void upSunVect(struct Sky * sky)
{
    sky->sunVect[0] = cos(sky->sunSite)*sin(sky->sunAzimuth);
    sky->sunVect[1] = cos(sky->sunSite)*cos(sky->sunAzimuth);
    sky->sunVect[2] = sin(sky->sunSite);
    //printf("sun: %f %f => [%f %f %f]\n",sky->sunSite,sky->sunAzimuth,
    //       sky->sunVect[0], sky->sunVect[1], sky->sunVect[2]);
}
