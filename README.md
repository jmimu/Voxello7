Voxello 7
=========

dbg.h from http://c.learncodethehardway.org

Choosing number of threads:
---------------------------
    export OMP_NUM_THREADS=4


Optimizations :
---------------
  * make one world for each thread? (cam and fc too?)
  * z_to_l and l_to_z
  * no more malloc and free for vinterval
  * vox column from top to down? (sky first)
  * simplify far column drawing
  * be able to clear only 1 column over n
  * be able to have different resolution in x and y (use window size ratio instead of render size ratio)
  
  
  * expand only necessary part of column
  * compress only empty space ?
  * faster clip_min
  * update only 1/2 col
