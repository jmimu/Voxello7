Voxello 7
=========

dbg.h from c.learncodethehardway.org
magicavoxel format from ephtracy.github.io

Choosing number of threads:
---------------------------
    export OMP_NUM_THREADS=4

bug: bottom of screen

TODO :
------
  * put all background part out of graph
  * stop using curr_compr_col

Optimizations :
---------------
  * make one world for each thread? (cam and fc too?)
  * z_to_l and l_to_z with fixed point?
  * simplify far column drawing
  * be able to have different resolution in x and y (use window size ratio instead of render size ratio)
  * add blur to far parts (shader?) => must use opengl : https://lazyfoo.net/tutorials/SDL/51_SDL_and_modern_opengl/index.php
  
  * compress only empty space ?
  * faster clip_min
  * update only 1/2 col

Improvements :
-------------
  * support multiple voxels objects => even for each vertical wall ?
  * support alpha voxel objects
  * make world edition thread-safe
  * how to update sub-worlds? reread all columns?
  * make collisions on sub-worlds?
