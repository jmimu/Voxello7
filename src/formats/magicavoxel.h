//================================
//  MagicaVoxel [12/09/2013]
//  Copyright(c) 2013 @ ephtracy. All rights reserved.
//================================

//================================
// Notice that this code is neither robust nor complete.
// It is only a sample code demonstrating
//     how to load current version .vox model from MagicaVoxel.
//================================


//c version by jmimu

//see: https://ephtracy.github.io/index.html?page=mv_vox_format
//see: https://voxel.codeplex.com/wikipage?title=Sample%20Codes


#ifndef __MV_MODEL__
#define __MV_MODEL__

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "../voxworld.h"

//================
// RGBA
//================
struct MV_RGBA {
    unsigned char r, g, b, a;
};


//================
// Model
//================
struct MV_Model {
    // size
    int sizex, sizey, sizez;
    
    // voxels
    int numVoxels;
    struct MV_Voxel *voxels;

    // palette
    bool isCustomPalette;
    struct MV_RGBA palette[ 256 ];
    
    // version
    int version;
};



struct MV_Model * LoadModel( const char *path );

void MV_Model_delete(struct MV_Model * model);

void VoxWorld_set_MV_Model_palette(struct VoxWorld * world,
    struct MV_Model * model);

bool VoxWorld_add_MV_Model(struct VoxWorld * world,
    struct MV_Model * model, int posx, int posy, int posz,bool use_empty);


#endif // __MV_MODEL__
