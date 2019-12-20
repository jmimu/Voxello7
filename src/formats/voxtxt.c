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
            if ((x+dx>0) && (x+dx<world->szX) && (y+dy>0) && (y+dy<world->szY) && (z+dz>0) && (z+dz<world->szZ))
                add_vox(world,x+dx,y+dy,z+dz,v);
        }else{
            check(false,"Wrong vox number: %ld~%ld: *%s*",i,nb,str);
        }
    }
    
error:
    // close file
    fclose( fp );
    printf("Finished loading %s.\n",path);
}

