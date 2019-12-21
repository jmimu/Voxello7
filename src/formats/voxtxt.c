#include "voxtxt.h"
#include "../dbg.h"


void add_vox(struct VoxWorld * world, int x, int y, int z, int v)
{
    voxworld_expand_col(world,x,y);
    world->curr_exp_col[z]=v+30;
    voxworld_compr_col(world);
    voxworld_write_compr_col(world,x,y);
}

struct VoxWorld * VoxWorld_create_from_txt(const char *path)
{
    #define MAXCHAR 100

    printf("Create world from txt model %s...\n",path);
    
    // open file
    FILE *fp = fopen( path, "r" );
    check(fp,"failed to open file");
    
    int xmin,xmax,ymin,ymax,zmin,zmax;
    long nb;
    char str[MAXCHAR];
    if (fgets(str, MAXCHAR, fp) != NULL)
    {
        if (sscanf(str, "%d %d %d %d %d %d %ld", &xmin,&xmax,&ymin,&ymax,&zmin,&zmax,&nb) != 7)
               check(false,"Bad 1st line format");
    }else{
        check(false,"File empty");
    }
    fclose( fp );
    
    struct VoxWorld * world = voxworld_create(xmax-xmin+1,ymax-ymin+1,zmax-zmin+1);
    VoxWorld_add_from_txt(world, path, -xmin, -ymin, -zmin);
    
    return world;
error:
    return NULL;
}

void VoxWorld_add_from_txt(struct VoxWorld * world, const char *path, int dx, int dy, int dz)
{
    #define MAXCHAR 100

    printf("Loading txt model %s...\n",path);
    
    // open file
    FILE *fp = fopen( path, "r" );
    check(fp,"failed to open file");
    
    int xmin,xmax,ymin,ymax,zmin,zmax;
    long nb,i;
    int x,y,z,v;
    int x_prev,y_prev;
    x_prev=-1;
    y_prev=-1;
    char str[MAXCHAR];
    
    if (fgets(str, MAXCHAR, fp) != NULL)
    {
        if (sscanf(str, "%d %d %d %d %d %d %ld", &xmin,&xmax,&ymin,&ymax,&zmin,&zmax,&nb) != 7)
               check(false,"Bad 1st line format");
    }else{
        check(false,"File empty");
    }
    
    for (i=0;i<nb-1;i++)
    {
        if (fgets(str, MAXCHAR, fp) != NULL)
        {
            if (sscanf(str, "%d %d %d %d", &x,&y,&z,&v) != 4)
               check(false,"Bad 1st line format");
            x+=dx;y+=dy;z+=dz;
            if ((x>=0) && (x<world->szX) && (y>=0) && (y<world->szY) && (z>=0) && (z<world->szZ))
            {
                //add_vox(world,x,y,z,v);
                if ((x!=x_prev)||(y!=y_prev))
                {
                    if (i>0)
                    {
                        voxworld_compr_col(world);
                        voxworld_write_compr_col(world,x_prev,y_prev);
                    }
                    voxworld_expand_col(world,x,y);
                }
                world->curr_exp_col[z]=v+30;
                
                x_prev=x;y_prev=y;
            }
            else
                printf("Out: %d %d %d\n",x+dx,y+dy,z+dz);
        }else{
            check(false,"Wrong vox number: %ld~%ld: *%s*",i,nb,str);
        }
    }
    voxworld_compr_col(world);
    voxworld_write_compr_col(world,x_prev,y_prev);
    
error:
    // close file
    fclose( fp );
    printf("Finished loading %s.\n",path);
}

