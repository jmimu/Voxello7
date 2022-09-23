#ifndef VOXTIF_H
#define VOXTIF_H

#include <stdio.h>
#include <stdlib.h>
#include "../voxworld.h"


/*******
 * importer for tif file
 * 1st: DTM, 2nd: RGB
 *******/


struct VoxWorld * VoxWorld_create_from_tif(const char *pathDTM, const char *pathRGB, double pix_size);

#endif // VOXTIF_H
