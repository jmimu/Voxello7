#ifndef VOXRENDER_H
#define VOXRENDER_H

#include "pt3d.h"
#include "voxworld.h"

struct VoxRender
{	
    struct VoxWorld *world;
    struct Pt3d cam;
    double center_ang_hz;
    double center_ang_vert;
    double fov_hz;//field of view hz
    double fov_vert;//field of view hz

    double zen2line_factor,zen2line_offset;
};


struct VoxRender * VoxRender_create(struct VoxWorld *_world,double _fov_hz);
void VoxRender_setCam(struct VoxRender * render,Pt3d _cam,double _center_ang_hz,double _center_ang_vert);
void VoxRender_render(struct VoxRender * render);


#endif // VOXRENDER_H
