#include <SDL2/SDL.h>
#include <stdio.h>
#include <math.h>

#include "graph.h"
#include "pt3d.h"
#include "dbg.h"
#include "voxworld.h"
#include "voxrender.h"

int main(int argc, char *argv[])
{
	(void) argc;
	(void) argv;
	bool result;
		
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

	struct Pt3d cam={2.50001,-10.001,0.5001};
	double focale=300;

	bool run=true;
	double t=0.0;

	int frame_couter=0;

	bool trace=false;
	
	struct VoxWorld * world=0;
	struct VoxRender * render=0;

	result=graph_init(800/2,600/2,800/2,600/2,"Voxello");
	check_debug(result,"Unable to open window...");
	
	world = voxworld_create(5,5,4);
	check_debug(world,"Unable to create world...");

	voxworld_init_empty_cube(world,2);
	//voxworld_printf(world);
	
	render=VoxRender_create(world,2);
	
	last_time = SDL_GetTicks();
	current_time = last_time;
	previous_fps_time=SDL_GetTicks()/1000;
	
	//SDL_EnableKeyRepeat(10, 10);

	SDL_SetRelativeMouseMode(SDL_TRUE);


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
						case SDLK_w:
							trace=true;
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

		if (key_g)
		{
			//add_to_cam=add_to_cam.mult(rendering.m_cam_orient);
			add(&cam,(struct Pt3d){speed*cos(angleZ),-speed*sin(angleZ),0});
		}
		if (key_d)
		{
			//add_to_cam=add_to_cam.mult(rendering.m_cam_orient);
			add(&cam,(struct Pt3d){-speed*cos(angleZ),speed*sin(angleZ),0});
		}
		if (key_f)
		{
			//add_to_cam=add_to_cam.mult(rendering.m_cam_orient);
			add(&cam,(struct Pt3d){-speed*sin(angleZ),-speed*cos(angleZ),0});
		}
		if (key_r)
		{
			//add_to_cam=add_to_cam.mult(rendering.m_cam_orient);
			add(&cam,(struct Pt3d){speed*sin(angleZ),speed*cos(angleZ),0});
		}
		if (key_e)
		{
			//add_to_cam=add_to_cam.mult(rendering.m_cam_orient);
			add(&cam,(struct Pt3d){0,0,speed});
		}
		if (key_t)
		{
			//add_to_cam=add_to_cam.mult(rendering.m_cam_orient);
			add(&cam,(struct Pt3d){0,0,-speed});
		}

		t+=0.05;

		graph_start_frame();

		VoxRender_setCam(render,cam,angleZ,angleX);
		VoxRender_render(render,trace);
		if (trace) trace=false;

		//graph_test();

		graph_end_frame();
		//run=false;


		//----- timing -----
		if (previous_fps_time!=current_time/1000)
		{
			//~ printf("FPS: %d\n",fps);
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
	if (world) voxworld_delete(world);
	graph_close();

}

