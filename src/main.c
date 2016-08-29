#include <SDL2/SDL.h>
#include <stdio.h>
#include <math.h>

#include "graph.h"
#include "pt3d.h"
#include "trigo.h"
#include "dbg.h"
#include "voxworld.h"
#include "voxrender.h"
#include "sprite.h"
#include "raster.h"
#include "formats/magicavoxel.h"

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
	double speed=0;//calculated with last frame duration

	struct Pt3d cam={2.50001,-5.001,2.001};
	double focale=300;

	bool run=true;
	long t=0;

	int frame_couter=0;

	bool trace=false;
	
	struct VoxWorld * world=0;
	struct VoxRender * render=0;

	//result=graph_init(640,480,640/2,480/2,"Voxello");
	result=graph_init(800,600,800,600,"Voxello");
	check_debug(result,"Unable to open window...");
	
	world = voxworld_create(2500,2500,120);
	//world = voxworld_create(6,6,6);
	check_debug(world,"Unable to create world...");
	cam.x=world->szX/3+0.001;
	//cam.y=world->szY/2+0.001;
	cam.z=1.5*world->szZ/2+0.001;

	//voxworld_init_empty_cube(world,2);
	//voxworld_init_full_cube(world);
	voxworld_init_land(world);
	//voxworld_init_stairs(world);
	//voxworld_printf(world);

	struct MV_Model * model = LoadModel( "data/castle.vox" );
	VoxWorld_add_MV_Model(world,model,20,20,15,0);
	VoxWorld_set_MV_Model_palette(world,model);
	MV_Model_delete(model);
	model = LoadModel( "data/monu1.vox" );
	VoxWorld_add_MV_Model(world,model,100,20,0,0);
	MV_Model_delete(model);
	model = LoadModel( "data/monu9.vox" );
	VoxWorld_add_MV_Model(world,model,100,100,2,0);
	MV_Model_delete(model);
	model = LoadModel( "data/ephtracy.vox" );
	VoxWorld_add_MV_Model(world,model,20,100,20,0);
	MV_Model_delete(model);
	
	
	render=voxrender_create(world,30);
	printf("Sizeof VoxRay: %ld\n",sizeof(struct VoxRay));
	
	last_time = SDL_GetTicks();
	current_time = last_time;
	previous_fps_time=SDL_GetTicks()/1000;
	
	//SDL_EnableKeyRepeat(10, 10);

	SDL_SetRelativeMouseMode(SDL_TRUE);

	struct Anim* anim1=anim_create(1);
	anim_add_raster(anim1,raster_load("data/run1.png"));
	anim_add_raster(anim1,raster_load("data/run2.png"));
	anim_add_raster(anim1,raster_load("data/run3.png"));
	anim_add_raster(anim1,raster_load("data/run4.png"));
	anim_add_raster(anim1,raster_load("data/run5.png"));
	anim_add_raster(anim1,raster_load("data/run6.png"));
	anim_add_raster(anim1,raster_load("data/run7.png"));
	anim_add_raster(anim1,raster_load("data/run8.png"));
	anim_add_raster(anim1,raster_load("data/run9.png"));
	struct Sprite* sprite1=sprite_create("Toto",100,90,23,4,4,anim1);


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
			add(&cam,(struct Pt3d){speed*_cos(angleZ),-speed*_sin(angleZ),0});
		}
		if (key_d)
		{
			//add_to_cam=add_to_cam.mult(rendering.m_cam_orient);
			add(&cam,(struct Pt3d){-speed*_cos(angleZ),speed*_sin(angleZ),0});
		}
		if (key_f)
		{
			//add_to_cam=add_to_cam.mult(rendering.m_cam_orient);
			add(&cam,(struct Pt3d){-speed*_sin(angleZ),-speed*_cos(angleZ),0});
		}
		if (key_r)
		{
			//add_to_cam=add_to_cam.mult(rendering.m_cam_orient);
			add(&cam,(struct Pt3d){speed*_sin(angleZ),speed*_cos(angleZ),0});
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

		if (cam.z<0) cam.z=0;
		if (cam.z>world->szZ) cam.z=world->szZ;

		t++;

		//graph_start_frame();
		voxrender_setCam(render,cam,angleZ);
		voxrender_render(render,trace);
		//struct Pt3d proj=voxrender_proj(render,raster1p);
		//raster_draw(raster1,proj.x-(raster1->w>>2),proj.z-(raster1->h),proj.y*8);

		sprite_draw(render,sprite1);

		//graph_test();

		graph_end_frame();
		//run=false;

		if (trace)
		{
			ScreenshotBMP("out.bmp");
			trace=false;
		}

		//----- timing -----
		if (previous_fps_time!=current_time/1000)
		{
			printf("FPS: %d\n",fps);
			previous_fps_time=current_time/1000;
			fps=0;
		}
		current_time = SDL_GetTicks();
		ellapsed_time = current_time - last_time;
		speed=ellapsed_time/20.0;
		last_time = current_time;
		ellapsed_time = SDL_GetTicks() - start_time;
		anim_frame(anim1,ellapsed_time);
		if (ellapsed_time < 20)
		{
			//SDL_Delay(20 - ellapsed_time);
		}
		//----- end timing -----
	}

error:
	//raster_unloadall();//TODO
	free(anim1);
	free(sprite1);
	if (render) voxrender_delete(render);
	if (world) voxworld_delete(world);
	graph_close();

}

