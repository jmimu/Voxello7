Voxello 7
=========

dbg.h from http://c.learncodethehardway.org


Optimizations :
---------------
  * z_to_l and l_to_z
  * no more malloc and free for vinterval
  * vox column from top to down? (sky first)
  * simplify far column drawing
  
  
  * expand only necessary part of column
  * compress only empty space ?
  * faster clip_min
  * update only 1/2 col
