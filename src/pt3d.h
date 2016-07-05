//      Pt3d.h
//      
//      Copyright 2016 jmimu
//      
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//      
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//      
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
//      MA 02110-1301, USA.
//      
//      


#ifndef PT3D_H
#define PT3D_H

#include <math.h>

inline int sign_d(double x)
{
    if (x>0.000000001) return 1;
    if (x<-0.000000001) return -1;
    return 0;
}
inline int sign_i(int x)
{
    if (x>=1) return 1;
    if (x<=-1) return -1;
    return 0;
}

struct Pt3d
{
	double x;
	double y;
	double z;
};

void add(struct Pt3d *p1,struct Pt3d p2);


#endif /* PT3D_H */ 
