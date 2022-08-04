#ifndef SKY_H
#define SKY_H

#include <math.h>

struct Sky
{
    float sunSite; // rad, 0 is zenith
    float sunAzimuth; // rad, 0 is y+
    float sunVect[3];
};

struct Sky * createSky();
void upSunVect(struct Sky * sky);

#endif // SKY_H
