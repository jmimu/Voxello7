#include <SDL2/SDL.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>

#include "graph.h"
#include "pt3d.h"
#include "trigo.h"
#include "dbg.h"
#include "voxworld.h"
#include "voxrender.h"
#include "mob.h"
#include "sprite.h"
#include "raster.h"
#include "background.h"
#include "sky.h"
#include "formats/magicavoxel.h"
#include "formats/voxtxt.h"

void *filling(void* arg)
{
	printf("Start filling world\n");
	struct VoxWorld * world = (struct VoxWorld *)arg;

	//voxworld_init_land(world);

	struct MV_Model * model = LoadModel( "data/castle.vox" );
	VoxWorld_add_MV_Model(world,model,20,20,voxworld_get_ground_z(world,20,20),0);
	MV_Model_delete(model);
	model = LoadModel( "data/monu1.vox" );
	VoxWorld_add_MV_Model(world,model,100,20,voxworld_get_ground_z(world,100,20),0);
	MV_Model_delete(model);
	model = LoadModel( "data/monu9.vox" );
	VoxWorld_add_MV_Model(world,model,100,100,voxworld_get_ground_z(world,100,100),0);
	MV_Model_delete(model);
	model = LoadModel( "data/ephtracy.vox" );
	VoxWorld_add_MV_Model(world,model,20,100,voxworld_get_ground_z(world,20,100),0);
	MV_Model_delete(model);

	//voxworld_init_Menger(world);

	//voxworld_init_empty_cube(world,2);
	//VoxWorld_add_from_txt(world, "/tmp/vox.txt", 500,500,200);

	printf("Done filling world\n");
	pthread_exit(NULL);
}

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
	bool key_o=false;
	bool key_k=false;
	bool key_l=false;
	bool key_m=false;

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
	struct Background * background=0;
    struct Sky * sky = createSky();

    result=graph_init(1920,1080,1920/4,1080,"Voxello");
	//result=graph_init(640,480,640/2,480/2,"Voxello");
	//result=graph_init(800,600,800/1,600/1,"Voxello");
	check_debug(result,"Unable to open window...");

	if (argc>1)
	{
		world = VoxWorld_create_from_txt(argv[1]);
		//return 0;
	}
	else
	{
		world = voxworld_create(1000,1000,200);
		voxworld_init_land(world);
	}
	if (!world) //in case of wrong filename
	{
		world = voxworld_create(400,400,200);
		voxworld_init_empty_cube(world,2);
	}
	//world = voxworld_create(3*3*3*3*3*3,3*3*3*3*3*3,3*3*3*3*3*3);
	check_debug(world,"Unable to create world...");
	cam.x=world->szX/3+0.001;
	//cam.y=world->szY/2+0.001;
	cam.z=1.5*world->szZ/2+0.001;

	//for cave
	angleZ = PI/2;
	cam.x=-world->szZ/2+0.001;
	cam.y=world->szY/2+0.001;
	cam.z=world->szZ/2+0.001;

	//test quake speed
	#ifdef TESTQUAKESPEED
		cam.x=world->szY*1499.5/3000;
		cam.y=world->szY*2238.5/3000;
		cam.z=world->szY*415.5/3000;
		angleZ=PI/2;
		voxworld_print_col(world,world->szY*1643/3000,world->szY*2239/3000);
	#endif

	//voxworld_init_empty_cube(world,2);
	//voxworld_init_full_cube(world);
	//voxworld_init_land(world);
	//voxworld_init_stairs(world);
	//voxworld_init_cave(world);
	//voxworld_printf(world);

	//voxworld_init_rand(world);

	render=voxrender_create(world,14);
	printf("Sizeof VoxRay: %ld\n",sizeof(struct VoxRay));

	last_time = SDL_GetTicks();
	current_time = last_time;
	previous_fps_time=SDL_GetTicks()/1000;

	//SDL_SetRelativeMouseMode(SDL_TRUE); //desactivate for debug

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
	struct Sprite* sprite1 = NULL;
	if ((world->szX>100) && (world->szY>90))
		sprite1=sprite_create("Toto",100,90,voxworld_get_ground_z(world,100,90),4,4,anim1);

	struct Anim* anim_bullet=anim_create(1);
	anim_add_raster(anim_bullet,raster_load("data/bullet.png"));
	struct Sprite* sprite_bullet = NULL;
	sprite_bullet=sprite_create("Bullet",10,10,10,1,1,anim_bullet);
	struct Mob* mob_bullet = NULL;

	background=background_create("data/back2.jpg");


	//voxworld_init_land2(world);
	{//separate thread part
		int res;
		pthread_t a_thread;
		res = pthread_create(&a_thread, NULL, filling, (void*)world);
		if (res != 0)
		{
			perror("Thread creation failed!\n");
		}
	}

	cam.x=-26.472362;cam.y=618.663497;cam.z=0;angleZ=M_PI/2; //debug water

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
							printf("Cam: %f %f %f, ang %f\n", cam.x, cam.y, cam.z, angleZ);
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
						case SDLK_o:
							key_o=true;
							break;
						case SDLK_k:
							key_k=true;
							break;
						case SDLK_l:
							key_l=true;
							break;
						case SDLK_m:
							key_m=true;
							break;
						case SDLK_s: //recompile shaders
							createShader(graph.shader, "src/shaders/shader.vert", "src/shaders/shader.frag");
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
						case SDLK_o:
							key_o=false;
							break;
						case SDLK_k:
							key_k=false;
							break;
						case SDLK_l:
							key_l=false;
							break;
						case SDLK_m:
							key_m=false;
							break;
						default:
							break;
					}
					break;
				case SDL_MOUSEMOTION:
					angleX += event.motion.yrel*0.002;
					angleZ += event.motion.xrel*0.002;
					if (angleZ<0) angleZ+=2*PI;
					if (angleZ>2*PI) angleZ-=2*PI;
				break;
				case SDL_MOUSEWHEEL:
					if (event.wheel.y>0) focale*=1.1;
					if (event.wheel.y<0) focale/=1.1;
					//std::cout<<"Dist: "<<distance<<std::endl;
				break;
				case SDL_MOUSEBUTTONDOWN:
					if (mob_bullet) free(mob_bullet);
					mob_bullet=mob_create(sprite_bullet,100*_sin(angleZ),100*_cos(angleZ),0);
                    mob_bullet->spr->pos=cam;
				break;

			}
		}
		//SDL_WarpMouseInWindow(graph->get_window(),graph->get_window_w()/2,graph->get_window_h()/2);
		//while (SDL_PollEvent(&event)) ;

		if (key_g)
		{
			//add_to_cam=add_to_cam.mult(rendering.m_cam_orient);
			add(&cam,(struct Pt3d){speed*_cos(angleZ)*(cam.z+50)/50,-speed*_sin(angleZ)*(cam.z+50)/50,0});
		}
		if (key_d)
		{
			//add_to_cam=add_to_cam.mult(rendering.m_cam_orient);
			add(&cam,(struct Pt3d){-speed*_cos(angleZ)*(cam.z+50)/50,speed*_sin(angleZ)*(cam.z+50)/50,0});
		}
		if (key_f)
		{
			//add_to_cam=add_to_cam.mult(rendering.m_cam_orient);
			add(&cam,(struct Pt3d){-speed*_sin(angleZ)*(cam.z+50)/50,-speed*_cos(angleZ)*(cam.z+50)/50,0});
		}
		if (key_r)
		{
			//add_to_cam=add_to_cam.mult(rendering.m_cam_orient);
			add(&cam,(struct Pt3d){speed*_sin(angleZ)*(cam.z+50)/50,speed*_cos(angleZ)*(cam.z+50)/50,0});
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

		if (key_o)
		{
			sky->sunSite += 0.02;
			if (sky->sunSite>M_PI) sky->sunSite -= 2*M_PI;
			upSunVect(sky);
		}
		if (key_k)
		{
			sky->sunAzimuth -= 0.02;
			if (sky->sunAzimuth<-M_PI) sky->sunAzimuth += 2*M_PI;
			upSunVect(sky);
		}
		if (key_l)
		{
			sky->sunSite -= 0.02;
			if (sky->sunSite<-M_PI) sky->sunSite += 2*M_PI;
			upSunVect(sky);
		}
		if (key_m)
		{
			sky->sunAzimuth += 0.02;
			if (sky->sunAzimuth>M_PI) sky->sunAzimuth -= 2*M_PI;
			upSunVect(sky);
		}


		if (cam.z<0) cam.z=0;
		if (cam.z>world->szZ) cam.z=world->szZ;

		t++;

		graph_start_frame();
        //background_draw(background,angleZ-0.54,angleZ+0.54);
		voxrender_setCam(render,cam,angleZ);
		voxrender_render(render,trace);
		//struct Pt3d proj=voxrender_proj(render,raster1p);
		//raster_draw(raster1,proj.x-(raster1->w>>2),proj.z-(raster1->h),proj.y*8);

		if (sprite1) sprite_draw(render,sprite1);
		if (mob_bullet) mob_draw(render,mob_bullet);

		//graph_test();
		glUseProgram(graph.shader->shaderProgram); //before setting uniforms
		glUniform3fv(glGetUniformLocation(graph.shader->shaderProgram, "sunDir"), 1, sky->sunVect); //impossible to make glUniform3f work??
		float sunUni[3] = {sky->sunSite, sky->sunAzimuth, 0};
		glUniform3fv(glGetUniformLocation(graph.shader->shaderProgram, "sunAng"), 1, sunUni); //TODO: convert to px with focale etc.
		float camUni[3] = {angleX, angleZ, render->f / graph.render_w};
		glUniform3fv(glGetUniformLocation(graph.shader->shaderProgram, "camera"), 1, camUni); //TODO: convert to px with focale etc.
		graph_end_frame();
		//run=false;

		if (trace)
		{
			printf("Cam: %f %f %f %f\n",cam.x,cam.y,cam.z,angleZ);
			//ScreenshotBMP("out.bmp");
			trace=false;
		}

		//----- timing -----
		if (previous_fps_time!=current_time/1000)
		{
			printf("FPS: %d\n",fps);
			fflush(stdout);
			previous_fps_time=current_time/1000;
			fps=0;
		}
		current_time = SDL_GetTicks();
		ellapsed_time = current_time - last_time;
		speed=ellapsed_time/20.0;
		last_time = current_time;
		ellapsed_time = SDL_GetTicks() - start_time;
		anim_frame(anim1,ellapsed_time);
		if (mob_bullet)
		{
			mob_update(world,mob_bullet,ellapsed_time/1000.0);
			if (mob_bullet->toDestroy)
			{
				free(mob_bullet);
				mob_bullet=NULL;
				printf("Bullet destroyed\n");
			}
		}

		if (ellapsed_time < 20)
		{
			//SDL_Delay(20 - ellapsed_time);
		}
		//----- end timing -----
	}

error:
	//raster_unloadall();//TODO
	free(sky);
	free(anim1);
	if (sprite1) free(sprite1);
	if (render) voxrender_delete(render);
	if (world) voxworld_delete(world);
	if (background) background_delete(background);
	graph_close();

}

