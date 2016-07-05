#include <SDL2/SDL.h>
#include <stdio.h>
#include <math.h>

#include "graph.h"
#include "pt3d.h"
#include "dbg.h"
#include "voxworld.h"

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;
	bool result;
	
	printf("%d\n",sizeof(struct RLE_block));
	
    //inputs
    bool key_r=false;
    bool key_f=false;
    bool key_d=false;
    bool key_g=false;
    bool key_e=false;
    bool key_t=false;

    //----- events -----
    SDL_Event event;

    //----- timing -----
    uint32_t last_time;
    uint32_t current_time;
    uint32_t ellapsed_time,start_time;
    uint32_t previous_fps_time;
    int fps=0;

    double angleZ = 0.0000;
    double angleX = 0.0000;
    double speed=0.01;

    struct Pt3d cam={-10.0001,0.5001,0.5001};
    double focale=300;

    bool run=true;
    double t=0.0;

    int frame_couter=0;


    result=graph_init(800/2,600/2,800/2,600/2,"Voxello");
    check_debug(result,"Unable to open window...");
    
    last_time = SDL_GetTicks();
    current_time = last_time;
    previous_fps_time=SDL_GetTicks()/1000;
    
    //SDL_EnableKeyRepeat(10, 10);

    SDL_SetRelativeMouseMode(SDL_TRUE);


    /*VoxWorld world(3,1,3);
    world.init();
    VoxRender render(world,graph,M_PI/4);
*/
    /*VoxRay ray(world,graph,&render);
    ray.reinit(cam,M_PI/4,-M_PI/8,M_PI/8,0);
    for (int i=0;i<10;i++)
    {
        ray.findNextIntersection();
        ray.coutInfo();
    }*/

    while (run)
    {
        //run=false;
        frame_couter++;//for valgrind
        //if (frame_couter>20) break;

        fps++;

        start_time = SDL_GetTicks();
        while (SDL_PollEvent(&event))
        {

            switch(event.type)
            {
                case SDL_QUIT:
                    exit(0);
                break;

                case SDL_KEYDOWN:
                    switch(event.key.keysym.sym)
                    {
                        case SDLK_ESCAPE:
                            run=false;
                            break;
                        case SDLK_r:
                key_r=true;
                            break;
                        case SDLK_f:
                key_f=true;
                            break;
                        case SDLK_d:
                key_d=true;
                            break;
                        case SDLK_g:
                key_g=true;
                            break;
                        case SDLK_e:
                key_e=true;
                            break;
                        case SDLK_t:
                key_t=true;
                            break;
                        default:
                            break;
                    }
                    break;
                case SDL_KEYUP:
                    switch(event.key.keysym.sym)
                    {
                        case SDLK_r:
                key_r=false;
                            break;
                        case SDLK_f:
                key_f=false;
                            break;
                        case SDLK_d:
                key_d=false;
                            break;
                        case SDLK_g:
                key_g=false;
                            break;
                        case SDLK_e:
                key_e=false;
                            break;
                        case SDLK_t:
                key_t=false;
                            break;
                        default:
                            break;
                    }
                    break;
                case SDL_MOUSEMOTION:
                    angleX += event.motion.yrel*0.002;
                    angleZ += event.motion.xrel*0.002;

                break;
                case SDL_MOUSEWHEEL:
                    if (event.wheel.y>0) focale*=1.1;
                    if (event.wheel.y<0) focale/=1.1;
                    //std::cout<<"Dist: "<<distance<<std::endl;
                break;
                case SDL_MOUSEBUTTONDOWN:
                    break;

            }
        }
        //SDL_WarpMouseInWindow(graph->get_window(),graph->get_window_w()/2,graph->get_window_h()/2);
        //while (SDL_PollEvent(&event)) ;

        if (key_r)
        {
            //add_to_cam=add_to_cam.mult(rendering.m_cam_orient);
            add(&cam,(struct Pt3d){speed*cos(angleZ),speed*sin(angleZ),0});
        }
        if (key_f)
        {
            //add_to_cam=add_to_cam.mult(rendering.m_cam_orient);
            add(&cam,(struct Pt3d){-speed*cos(angleZ),-speed*sin(angleZ),0});
        }
        if (key_d)
        {
            //add_to_cam=add_to_cam.mult(rendering.m_cam_orient);
            add(&cam,(struct Pt3d){speed*cos(angleZ-M_PI/2),speed*sin(angleZ-M_PI/2),0});
        }
        if (key_g)
        {
            //add_to_cam=add_to_cam.mult(rendering.m_cam_orient);
            add(&cam,(struct Pt3d){-speed*cos(angleZ-M_PI/2),-speed*sin(angleZ-M_PI/2),0});
        }
        if (key_e)
        {
            //add_to_cam=add_to_cam.mult(rendering.m_cam_orient);
            add(&cam,(struct Pt3d){0,0,speed});
        }
        if (key_t)
        {
            //add_to_cam=add_to_cam.mult(rendering.m_cam_orient);
            add(&cam,(struct Pt3d){0,0,speed});
        }

        t+=0.05;

        graph_start_frame();

        /*render.setCam(cam,angleZ,angleX);
        render.render();*/
        graph_test();

        graph_end_frame();
        //run=false;


        //----- timing -----
        if (previous_fps_time!=current_time/1000)
        {
            printf("FPS: %d\n",fps);
            previous_fps_time=current_time/1000;
            fps=0;
        }
        current_time = SDL_GetTicks();
        ellapsed_time = current_time - last_time;
        last_time = current_time;
        ellapsed_time = SDL_GetTicks() - start_time;
        if (ellapsed_time < 20)
        {
            //SDL_Delay(20 - ellapsed_time);
        }
        //----- end timing -----
    }

error:
	graph_close();

}

