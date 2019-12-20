#ifndef VOXTXT_H
#define VOXTXT_H

#include <stdio.h>
#include <stdlib.h>
#include "../voxworld.h"


/*******
 * importer for txt files
 * format:
 *   - x_min_o,x_max_o,y_min_o,y_max_o,z_min_o,z_max_o,nb
 *   - list of: x y z val
 *******/

void VoxWorld_add_from_txt(struct VoxWorld * world, const char *path, int dx, int dy, int dz);

struct VoxWorld * VoxWorld_create_from_txt(const char *path);

#endif // VOXTXT_H
