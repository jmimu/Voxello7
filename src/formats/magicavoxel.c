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


#include "magicavoxel.h"

#include "../dbg.h"



// magic number
int MV_ID( int a, int b, int c, int d );


//================
// Voxel
//================
struct MV_Voxel {
    unsigned char x, y, z, colorIndex;
};



int ReadInt( FILE *fp );


struct MV_Model * MV_Model_create();


struct chunk_t {
    int id;
    int contentSize;
    int childrenSize;
    long end;
};


void ReadChunk( FILE *fp, struct chunk_t *chunk );

bool ReadModelFile( struct MV_Model * model, FILE *fp );


//--------------------------------------


// magic number
int MV_ID( int a, int b, int c, int d ) {
    return ( a ) | ( b << 8 ) | ( c << 16 ) | ( d << 24 );
}

int ReadInt( FILE *fp ) {
    int v = 0;
    int n=fread( &v, 4, 1, fp );
    check(n==1,"Error reading int!");

    return v;
error:
    return -1;
}

void MV_Model_delete(struct MV_Model * model) {
    if ( model->voxels ) {
        free(model->voxels);
        model->voxels = NULL;
    }
    model->numVoxels = 0;
    
    model->sizex = model->sizey = model->sizez = 0;
    
    model->isCustomPalette = false;

    model->version = 0;
    free(model);
}

struct MV_Model * MV_Model_create()
{
    struct MV_Model * model = (struct MV_Model *)malloc(sizeof(struct MV_Model));
    model->sizex=0;
    model->sizey=0;
    model->sizez=0;
    model->numVoxels=0;
    model->voxels=0;
    model->isCustomPalette=false;
    model->version=0;
    return model;
}


void ReadChunk( FILE *fp, struct chunk_t *chunk ) {
    // read chunk
    chunk->id = ReadInt( fp );
    chunk->contentSize  = ReadInt( fp );
    chunk->childrenSize = ReadInt( fp );
    
    // end of chunk : used for skipping the whole chunk
    chunk->end = ftell( fp ) + chunk->contentSize + chunk->childrenSize;
    
    // print chunk info
    const char *c = ( const char * )( &chunk->id );
    printf( "[Log] MV_VoxelModel :: Chunk : %c%c%c%c : %d %d\n",
           c[0], c[1], c[2], c[3],
           chunk->contentSize, chunk->childrenSize
           );
}

bool ReadModelFile( struct MV_Model * model, FILE *fp ) {
    const int MV_VERSION = 150;
    
    const int ID_VOX  = MV_ID( 'V', 'O', 'X', ' ' );
    const int ID_MAIN = MV_ID( 'M', 'A', 'I', 'N' );
    const int ID_SIZE = MV_ID( 'S', 'I', 'Z', 'E' );
    const int ID_XYZI = MV_ID( 'X', 'Y', 'Z', 'I' );
    const int ID_RGBA = MV_ID( 'R', 'G', 'B', 'A' );

    int n;//to ckeck fread
   
    // magic number
    int magic = ReadInt( fp );
    check(magic == ID_VOX,"magic number does not match");
    
    // version
    model->version = ReadInt( fp );
    check( model->version == MV_VERSION,"version does not match");
    
    // main chunk
    struct chunk_t mainChunk;
    ReadChunk( fp, &mainChunk );
    check(mainChunk.id == ID_MAIN,"main chunk is not found");
    
    // skip content of main chunk
    fseek( fp, mainChunk.contentSize, SEEK_CUR );
    
    // read children chunks
    while ( ftell( fp ) < mainChunk.end ) {
        // read chunk header
        struct chunk_t sub;
        ReadChunk( fp, &sub );
        
        if ( sub.id == ID_SIZE ) {
            // size
            model->sizex = ReadInt( fp );
            model->sizey = ReadInt( fp );
            model->sizez = ReadInt( fp );
        }
        else if ( sub.id == ID_XYZI ) {
            // numVoxels
            model->numVoxels = ReadInt( fp );
            check(model->numVoxels >= 0,"negative number of voxels");
            
            // voxels
            if ( model->numVoxels > 0 ) {
                model->voxels = (struct MV_Voxel*)
                                malloc(model->numVoxels*sizeof(struct MV_Voxel));
                n=fread( model->voxels, sizeof( struct MV_Voxel ), model->numVoxels, fp );
                check(n==model->numVoxels,"Error reading voxels!");
            }
        }
        else if ( sub.id == ID_RGBA ) {
            // last color is not used, so we only need to read 255 colors
            model->isCustomPalette = true;
            n=fread( model->palette + 1, sizeof( struct MV_RGBA ), 255, fp );
            check(n==255,"Error reading palette!");

            // NOTICE : skip the last reserved color
            struct MV_RGBA reserved;
            n=fread( &reserved, sizeof( struct MV_RGBA ), 1, fp );
        }

        // skip unread bytes of current chunk or the whole unused chunk
        fseek( fp, sub.end, SEEK_SET );
    }
    
    // print model info
    printf( "[Log] MV_VoxelModel :: Model : %d %d %d : %d\n",
           model->sizex, model->sizey, model->sizez, model->numVoxels
           );
    
    return true;

error:
    return false;
}



struct MV_Model * LoadModel( const char *path )
{
    printf("Loading model %s...\n",path);
    struct MV_Model * model = MV_Model_create();
    
    // open file
    FILE *fp = fopen( path, "rb" );
    check(fp,"failed to open file");
    
    // read file
    bool success = ReadModelFile( model, fp );
    
    // close file
    fclose( fp );
    
    // if failed, free invalid data
    if ( !success ) {
        MV_Model_delete(model);
        return 0;
    }
    
    return model;

error:
    return 0;
}


/*    int sizex, sizey, sizez;
    // voxels
    int numVoxels;
    struct MV_Voxel *voxels;
    // palette
    bool isCustomPalette;
    struct MV_RGBA palette[ 256 ];
    // version
    int version;*/

void VoxWorld_set_MV_Model_palette(struct VoxWorld * world,
    struct MV_Model * model)
{
    int i;
    if(!model)
    {
        printf("Error: MV_Model NULL!\n");
        return;
    }
    for (i=0;i<255;i++)
    {
        world->colorMap[i]= (model->palette[i].a<<24)+
                            (model->palette[i].r<<16)+
                            (model->palette[i].g<<8)+
                            (model->palette[i].b<<0);
    }
}


bool VoxWorld_add_MV_Model(struct VoxWorld * world,
    struct MV_Model * model, int posx, int posy, int posz,
    bool use_empty)
{
    int i,j,x,y,z,xw,yw,zw;
    if(!model)
    {
        printf("Error: MV_Model NULL!\n");
        return false;
    }

    if ((posx<0)||(posx>world->szX-model->sizex)||
        (posy<0)||(posy>world->szY-model->sizey)||
        (posz<0)||(posz>world->szZ-model->sizez))
    {
        printf("Error: MV_Model out of world!\n");
        return false;
    }

    //create a coarse 3d char array
    int sizexy=model->sizex*model->sizey;
    uint8_t* coarse = (uint8_t*)malloc(sizexy*model->sizez*sizeof(uint8_t));
    memset(coarse,EMPTY,sizexy*model->sizez*sizeof(uint8_t));
    for (i=0;i<model->numVoxels;i++)
    {
        x=model->voxels[i].x;
        y=model->voxels[i].y;
        z=model->voxels[i].z;
        j=x*sizexy+y*model->sizex+z;
        coarse[j]=model->voxels[i].colorIndex;
    }

    //copy it into world
    for (x=0;x<model->sizex;x++)
    {
        xw=x+posx;
        for (y=0;y<model->sizey;y++)
        {
            yw=y+posy;
            voxworld_expand_col(world,xw,yw);
            j=x*sizexy+y*model->sizex;
            for (z=0;z<model->sizez;z++)
            {
                zw=z+posz;
                if (coarse[j]!=EMPTY)
                    world->curr_exp_col[zw]=coarse[j];
                else if (use_empty)
                    world->curr_exp_col[zw]=EMPTY;
                j++;
            }
            voxworld_compr_col(world);
            voxworld_write_compr_col(world,xw,yw);
        }
    }
    free(coarse);
    return true;
}

