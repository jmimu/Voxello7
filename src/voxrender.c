#include "voxrender.h"
#include "graph.h"

int zen2line(struct VoxRender * render,double zen);

struct VoxRender * VoxRender_create(struct VoxWorld *_world,double _fov_hz)
{
	struct VoxRender *render = (struct VoxRender *) malloc(sizeof(struct VoxRender));
    render->world=_world;
    render->fov_hz=_fov_hz;
    render->fov_vert=_fov_hz*graph.render_h/graph.render_w;
    
    //create rays for every display column
    /*for (int i=0;i<mGraph.get_render_w();i++)
    {
        mRays.push_back(VoxRay(mWorld,mGraph,this));
    }*/
    
    return render;
}

void VoxRender_setCam(struct VoxRender * render,Pt3d _cam,double _center_ang_hz,double _center_ang_vert)
{
    render->cam=_cam;
    render->center_ang_hz=_center_ang_hz;
    render->center_ang_vert=_center_ang_vert;

    //compute agular steps
    double current_hz_angle=render->center_ang_hz-render->fov_hz/2;
    double step_hz_angle=render->fov_hz/graph.render_w;
    double start_vert_angle=render->center_ang_vert-render->fov_vert/2;
    double stop_vert_angle=render->center_ang_vert+render->fov_vert/2;

    //zen2line formula is:
    //l(zen)=H-H*(zen-start_vert_angle)/(stop_vert_angle-start_vert_angle)
    //=zen*(-H/(stop_vert_angle-start_vert_angle))+H(1+start_vert_angle/(stop_vert_angle-start_vert_angle))
    //=zen*zen2line_factor+zen2line_offset
    render->zen2line_factor=(-graph.render_h/(stop_vert_angle-start_vert_angle));
    render->zen2line_offset=graph.render_h*(1+start_vert_angle/(stop_vert_angle-start_vert_angle));


    /*int currentColumn=0;

    for (auto &ray:mRays)
    {
        ray.reinit(mCam, current_hz_angle,start_vert_angle,stop_vert_angle, currentColumn);
        current_hz_angle+=step_hz_angle;
        currentColumn++;
    }*/
}


void VoxRender_render(struct VoxRender * render)
{
    //mRays[5].draw();
    /*for (auto &ray:mRays)
    {
        ray.draw();
    }*/

}


int zen2line(struct VoxRender * render,double zen)
{
    return zen*render->zen2line_factor+render->zen2line_offset;
}
