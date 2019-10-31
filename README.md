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

Optimizations :
---------------
  * make one world for each thread? (cam and fc too?)
  * z_to_l and l_to_z with fixed point?
  * vox column from top to down? (sky first)
  * simplify far column drawing
  * be able to have different resolution in x and y (use window size ratio instead of render size ratio)
  * add blur to far parts (shader?) => must use opengl : https://lazyfoo.net/tutorials/SDL/51_SDL_and_modern_opengl/index.php
  
  * expand only necessary part of column
  * record last and first full vox of each col
  * compress only empty space ?
  * faster clip_min
  * update only 1/2 col

Improvements :
-------------
  * support multiple voxels objects
  * support alpha voxel objects
  * make world edition thread-safe
