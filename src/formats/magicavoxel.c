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
    if (!model->isCustomPalette)
    {
        for (i=0;i<255;i++)
            world->colorMap[i]= mv_default_palette[i];
    }else{
        for (i=0;i<255;i++)
        {
            world->colorMap[i]= (model->palette[i].a<<24)+
                                (model->palette[i].r<<16)+
                                (model->palette[i].g<<8)+
                                (model->palette[i].b<<0);
        }
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
    int sizeyz=model->sizey*model->sizez;
    uint8_t* coarse = (uint8_t*)malloc(sizeyz*model->sizex*sizeof(uint8_t));
    memset(coarse,EMPTY,sizeyz*model->sizex*sizeof(uint8_t));
    for (i=0;i<model->numVoxels;i++)
    {
        x=model->voxels[i].x;
        y=model->voxels[i].y;
        z=model->voxels[i].z;
        j=x*sizeyz+y*model->sizez+z;
        //printf("(%d %d %d) (%d %d %d) i=%d, j=%d, sz=%d, sz=%d\n",
        //    model->sizex,model->sizey,model->sizez,
        //    x,y,z,i,j,model->numVoxels,sizexy*model->sizez);
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
            j=x*sizeyz+y*model->sizez;
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

const unsigned int mv_default_palette[ 256 ] = {
        0xffffffff, 0xffccffff, 0xff99ffff, 0xff66ffff, 0xff33ffff, 0xff00ffff,
    0xffffccff, 0xffccccff, 0xff99ccff, 0xff66ccff, 0xff33ccff, 0xff00ccff,
    0xffff99ff, 0xffcc99ff, 0xff9999ff, 0xff6699ff, 0xff3399ff, 0xff0099ff,
    0xffff66ff, 0xffcc66ff, 0xff9966ff, 0xff6666ff, 0xff3366ff, 0xff0066ff,
    0xffff33ff, 0xffcc33ff, 0xff9933ff, 0xff6633ff, 0xff3333ff, 0xff0033ff,
    0xffff00ff, 0xffcc00ff, 0xff9900ff, 0xff6600ff, 0xff3300ff, 0xff0000ff,
    0xffffffcc, 0xffccffcc, 0xff99ffcc, 0xff66ffcc, 0xff33ffcc, 0xff00ffcc,
    0xffffcccc, 0xffcccccc, 0xff99cccc, 0xff66cccc, 0xff33cccc, 0xff00cccc,
    0xffff99cc, 0xffcc99cc, 0xff9999cc, 0xff6699cc, 0xff3399cc, 0xff0099cc,
    0xffff66cc, 0xffcc66cc, 0xff9966cc, 0xff6666cc, 0xff3366cc, 0xff0066cc,
    0xffff33cc, 0xffcc33cc, 0xff9933cc, 0xff6633cc, 0xff3333cc, 0xff0033cc,
    0xffff00cc, 0xffcc00cc, 0xff9900cc, 0xff6600cc, 0xff3300cc, 0xff0000cc,
    0xffffff99, 0xffccff99, 0xff99ff99, 0xff66ff99, 0xff33ff99, 0xff00ff99,
    0xffffcc99, 0xffcccc99, 0xff99cc99, 0xff66cc99, 0xff33cc99, 0xff00cc99,
    0xffff9999, 0xffcc9999, 0xff999999, 0xff669999, 0xff339999, 0xff009999,
    0xffff6699, 0xffcc6699, 0xff996699, 0xff666699, 0xff336699, 0xff006699,
    0xffff3399, 0xffcc3399, 0xff993399, 0xff663399, 0xff333399, 0xff003399,
    0xffff0099, 0xffcc0099, 0xff990099, 0xff660099, 0xff330099, 0xff000099,
    0xffffff66, 0xffccff66, 0xff99ff66, 0xff66ff66, 0xff33ff66, 0xff00ff66,
    0xffffcc66, 0xffcccc66, 0xff99cc66, 0xff66cc66, 0xff33cc66, 0xff00cc66,
    0xffff9966, 0xffcc9966, 0xff999966, 0xff669966, 0xff339966, 0xff009966,
    0xffff6666, 0xffcc6666, 0xff996666, 0xff666666, 0xff336666, 0xff006666,
    0xffff3366, 0xffcc3366, 0xff993366, 0xff663366, 0xff333366, 0xff003366,
    0xffff0066, 0xffcc0066, 0xff990066, 0xff660066, 0xff330066, 0xff000066,
    0xffffff33, 0xffccff33, 0xff99ff33, 0xff66ff33, 0xff33ff33, 0xff00ff33,
    0xffffcc33, 0xffcccc33, 0xff99cc33, 0xff66cc33, 0xff33cc33, 0xff00cc33,
    0xffff9933, 0xffcc9933, 0xff999933, 0xff669933, 0xff339933, 0xff009933,
    0xffff6633, 0xffcc6633, 0xff996633, 0xff666633, 0xff336633, 0xff006633,
    0xffff3333, 0xffcc3333, 0xff993333, 0xff663333, 0xff333333, 0xff003333,
    0xffff0033, 0xffcc0033, 0xff990033, 0xff660033, 0xff330033, 0xff000033,
    0xffffff00, 0xffccff00, 0xff99ff00, 0xff66ff00, 0xff33ff00, 0xff00ff00,
    0xffffcc00, 0xffcccc00, 0xff99cc00, 0xff66cc00, 0xff33cc00, 0xff00cc00,
    0xffff9900, 0xffcc9900, 0xff999900, 0xff669900, 0xff339900, 0xff009900,
    0xffff6600, 0xffcc6600, 0xff996600, 0xff666600, 0xff336600, 0xff006600,
    0xffff3300, 0xffcc3300, 0xff993300, 0xff663300, 0xff333300, 0xff003300,
    0xffff0000, 0xffcc0000, 0xff990000, 0xff660000, 0xff330000, 0xff0000ee,
    0xff0000dd, 0xff0000bb, 0xff0000aa, 0xff000088, 0xff000077, 0xff000055,
    0xff000044, 0xff000022, 0xff000011, 0xff00ee00, 0xff00dd00, 0xff00bb00,
    0xff00aa00, 0xff008800, 0xff007700, 0xff005500, 0xff004400, 0xff002200,
    0xff001100, 0xffee0000, 0xffdd0000, 0xffbb0000, 0xffaa0000, 0xff880000,
    0xff770000, 0xff550000, 0xff440000, 0xff220000, 0xff110000, 0xffeeeeee,
    0xffdddddd, 0xffbbbbbb, 0xffaaaaaa, 0xff888888, 0xff777777, 0xff555555,
    0xff444444, 0xff222222, 0xff111111, 0xff000000,
};