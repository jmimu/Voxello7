import bpy
from bpy import context
from mathutils import *
from math import *
C = bpy.context
D = bpy.data

#only current object is voxelized. Join meshes to have a complex scene.
#Must be in object mode, with no faces selected in edit mode.
#TODO: auto light & camera remove ?

class NotTriangulated(Exception):
    pass

#bpy.ops.object.select_all(action='SELECT')
#bpy.ops.object.delete(use_global=False)

#bpy.ops.mesh.primitive_cone_add(location=(0, 0, 0), vertices=7)
#bpy.ops.mesh.primitive_torus_add(major_segments=8, minor_segments=6,align='WORLD', location=(0, 0, 0), rotation=(0, 0, 0), major_radius=1, minor_radius=0.25, abso_major_rad=1.25, abso_minor_rad=0.75, )

bpy.ops.object.editmode_toggle()
bpy.ops.mesh.quads_convert_to_tris(quad_method='BEAUTY', ngon_method='BEAUTY')
bpy.ops.object.editmode_toggle()

obj = context.active_object

#output dim
use_fix_vox_size = False
vox_max_size = 3000 #in 1 dimension
vox_size=0.1

add_cubes = False

all_vox = [] #list of tuples : (x,y,z,c)

for f in obj.data.polygons:
    if len(f.vertices)>3:
        raise NotTriangulated

x_min_o = float("inf")
x_max_o = -float("inf")
y_min_o = float("inf")
y_max_o = -float("inf")
z_min_o = float("inf")
z_max_o = -float("inf")
for v in obj.data.vertices:
    co = v.co
    if co.x<x_min_o:
        x_min_o=co.x
    if co.x>x_max_o:
        x_max_o=co.x
    if co.y<y_min_o:
        y_min_o=co.y
    if co.y>y_max_o:
        y_max_o=co.y
    if co.z<z_min_o:
        z_min_o=co.z
    if co.z>z_max_o:
        z_max_o=co.z
#print("Range: ",x_min_o," - ",x_max_o,"; ",y_min_o," - ",y_max_o,"; ",z_min_o," - ",z_max_o)

if not use_fix_vox_size:
    vox_size= max( (x_max_o-x_min_o,y_max_o-y_min_o,z_max_o-z_min_o) ) / vox_max_size

x_min_o=floor(x_min_o/vox_size)
x_max_o= ceil(x_max_o/vox_size)
y_min_o=floor(y_min_o/vox_size)
y_max_o= ceil(y_max_o/vox_size)
z_min_o=floor(z_min_o/vox_size)
z_max_o= ceil(z_max_o/vox_size)

nb_faces = len(obj.data.polygons)
num_face = 0
for f in obj.data.polygons:
    print(int(100.0*num_face/nb_faces),"%");
    x_min_f = float("inf")
    x_max_f = -float("inf")
    y_min_f = float("inf")
    y_max_f = -float("inf")
    z_min_f = float("inf")
    z_max_f = -float("inf")
    
    ptA=obj.data.vertices[f.vertices[0]].co/vox_size
    ptB=obj.data.vertices[f.vertices[1]].co/vox_size
    ptC=obj.data.vertices[f.vertices[2]].co/vox_size
    vectU = ptB - ptA
    vectV = ptC - ptA
    vectW = Vector(( vectU.y*vectV.z-vectU.z*vectV.y, vectU.z*vectV.x-vectU.x*vectV.z, vectU.x*vectV.y-vectU.y*vectV.x ))
    vectW.normalize()
    mat = Matrix( [vectU, vectV, vectW] )
    mat.transpose()

    #print("points:\n",ptA,"\n", ptB,"\n", ptC,"\n")
    #print("vects:\n",vectU,"\n", vectV,"\n", vectW,"\n", mat)
    
    mat_inv = mat.copy()
    mat_inv.invert_safe()
    #print(mat_inv)

    for vertex in f.vertices :
        co = obj.data.vertices[vertex].co
        if co.x<x_min_f:
            x_min_f=co.x
        if co.x>x_max_f:
            x_max_f=co.x
        if co.y<y_min_f:
            y_min_f=co.y
        if co.y>y_max_f:
            y_max_f=co.y
        if co.z<z_min_f:
            z_min_f=co.z
        if co.z>z_max_f:
            z_max_f=co.z

    #print("Range: ",x_min_f," - ",x_max_f,"; ",y_min_f," - ",y_max_f,"; ",z_min_f," - ",z_max_f)

    x_min_f=floor(x_min_f/vox_size)
    x_max_f= ceil(x_max_f/vox_size)
    y_min_f=floor(y_min_f/vox_size)
    y_max_f= ceil(y_max_f/vox_size)
    z_min_f=floor(z_min_f/vox_size)
    z_max_f= ceil(z_max_f/vox_size)

    for x in range(x_min_f,x_max_f+1):
        for y in range(y_min_f,y_max_f+1):
            for z in range(z_min_f,z_max_f+1):
                ptM = Vector( [x,y,z] )
                vectAM = ptM - ptA
                decomp = mat_inv @ vectAM
                check = mat @ decomp
                #print("decomp: ",decomp, ptM, vectAM)
                if (decomp.x+decomp.y<1.05) and (-0.05<decomp.x) and (-0.05<decomp.y) and (-1.005<decomp.z) and (decomp.z<0.005):
                    #print("add")
                    if (add_cubes):
                        bpy.ops.mesh.primitive_cube_add(location=(x,y,z),size=1)
                    all_vox.append( (x,y,z,10) )
    num_face+=1

filename="/tmp/vox.txt"
print("Write to",filename)
fout=open(filename,"w")
#range
fout.write("%d %d %d %d %d %d %d\n"%(x_min_o,x_max_o,y_min_o,y_max_o,z_min_o,z_max_o,len(all_vox)))
#data
for v in all_vox:
    fout.write("%d %d %d %d\n"%v)
fout.close()
