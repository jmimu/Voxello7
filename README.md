Voxello 7
=========

dbg.h from c.learncodethehardway.org

magicavoxel format from ephtracy.github.io

Dependencies
------------

libSDL2 libSDL2image

Compilation
-----------

    make

Run
---

    bin/voxello7 [vox_file.txt]


To choose number of threads
---------------------------
    export OMP_NUM_THREADS=4

Features
--------
  * Colors very bright have no shades, looking as if emitting light!


TODO
----
  * put all background part out of graph
  * stop using curr_compr_col

Optimizations
-------------
  * make one world for each thread? (cam and fc too?)
  * z_to_l and l_to_z with fixed point?
  * simplify far column drawing
  * be able to have different resolution in x and y (use window size ratio instead of render size ratio)
  * add blur to far parts (shader?) => must use opengl: https://lazyfoo.net/tutorials/SDL/51_SDL_and_modern_opengl/index.php
  
  * compress only empty space?
  * faster clip_min
  * update only 1/2 col

Improvements
------------
  * support multiple voxels objects => even for each vertical wall?
  * support alpha voxel objects
  * make world edition and collision check thread-safe
  * how to update sub-worlds? reread all columns?
  * make collisions on sub-worlds?
