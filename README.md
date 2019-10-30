Voxello 7
=========

dbg.h from c.learncodethehardway.org
magicavoxel format from ephtracy.github.io

Choosing number of threads:
---------------------------
    export OMP_NUM_THREADS=4

bug: bottom of screen

Optimizations :
---------------
  * make one world for each thread? (cam and fc too?)
  * z_to_l and l_to_z with fixed point?
  * vox column from top to down? (sky first)
  * simplify far column drawing
  * be able to have different resolution in x and y (use window size ratio instead of render size ratio)
  
  * expand only necessary part of column
  * record last and first full vox of each col
  * compress only empty space ?
  * faster clip_min
  * update only 1/2 col
