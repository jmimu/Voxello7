#ifndef SKY_H
#define SKY_H

#include <math.h>

# define M_PI		3.14159265358979323846	/* pi */
# define M_PI_2		1.57079632679489661923	/* pi/2 */

struct Sky
{
    float sunSite; // rad, 0 is hz
    float sunAzimuth; // rad, 0 is y+
    float sunVect[3];
};

struct Sky * createSky();
void upSunVect(struct Sky * sky);

#endif // SKY_H
