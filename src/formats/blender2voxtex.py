import bpy
from bpy import context
from mathutils import *
from math import *

from mathutils.interpolate import poly_3d_calc
from bpy.types import Scene, Mesh, MeshPolygon, Image


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

#seems to need to apply triangulate modifier...
bpy.ops.object.editmode_toggle()
bpy.ops.mesh.quads_convert_to_tris(quad_method='BEAUTY', ngon_method='BEAUTY')
bpy.ops.object.editmode_toggle()

#is said to need uv unwrap but this make quake maps loose texture

obj = context.active_object
mesh = obj.data

#output dim
use_fix_vox_size = False
vox_max_size = 2000 #in 1 dimension
vox_size=0.1

add_cubes = False

all_vox = [] #list of tuples : (x,y,z,c)

for f in obj.data.polygons:
    if len(f.vertices)>3:
        raise NotTriangulated

#these functions come from https://blender.stackexchange.com/questions/139384/get-rgb-value-of-texture-from-face-on-mesh
def getUVPixelColor(mesh:Mesh, face, point:Vector, image:Image):
    """ get RGBA value for point in UV image at specified face index
    mesh     -- target mesh (must be uv unwrapped)
    face     -- face in target mesh to grab texture color from
    point    -- location (in 3D space on the specified face) to grab texture color from
    image    -- UV image used as texture for 'mesh' object
    """
    # ensure image contains at least one pixel
    assert image is not None and image.pixels is not None and len(image.pixels) > 0
    # get closest material using UV map
    scn = bpy.context.scene
    # get uv coordinate based on nearest face intersection
    uv_coord = getUVCoord(mesh, face, point, image)
    # retrieve rgba value at uv coordinate
    rgba = getPixel(image, uv_coord)
    return rgba


def getUVCoord(mesh:Mesh, face:MeshPolygon, point:Vector, image:Image):
    """ returns UV coordinate of target point in source mesh image texture
    mesh  -- mesh data from source object
    face  -- face object from mesh
    point -- coordinate of target point on source mesh
    image -- image texture for source mesh
    """
    # get active uv layer data
    uv_layer = mesh.uv_layers.active
    assert uv_layer is not None # ensures mesh has a uv map
    uv = uv_layer.data
    # get 3D coordinates of face's vertices
    lco = [mesh.vertices[i].co for i in face.vertices]
    # get uv coordinates of face's vertices
    luv = [uv[i].uv for i in face.loop_indices]
    # calculate barycentric weights for point
    lwts = poly_3d_calc(lco, point)
    # multiply barycentric weights by uv coordinates
    uv_loc = sum((p*w for p,w in zip(luv,lwts)), Vector((0,0)))
    # ensure uv_loc is in range(0,1)
    # TODO: possibly approach this differently? currently, uv verts that are outside the image are wrapped to the other side
    uv_loc = Vector((uv_loc[0] % 1, uv_loc[1] % 1))
    # convert uv_loc in range(0,1) to uv coordinate
    image_size_x, image_size_y = image.size
    x_co = round(uv_loc.x * (image_size_x - 1))
    y_co = round(uv_loc.y * (image_size_y - 1))
    uv_coord = (x_co, y_co)
    # return resulting uv coordinate
    return Vector(uv_coord)

# reference: https://svn.blender.org/svnroot/bf-extensions/trunk/py/scripts/addons/uv_bake_texture_to_vcols.py
def getPixel(img, uv_coord):
    """ get RGBA value for specified coordinate in UV image
    pixels    -- list of pixel data from UV texture image
    uv_coord  -- UV coordinate of desired pixel value
    """
    uv_pixels = img.pixels # Accessing pixels directly is quite slow. Copy to new array and pass as an argument for massive performance-gain if you plan to run this function many times on the same image (img.pixels[:]).
    pixelNumber = (img.size[0] * int(uv_coord.y)) + int(uv_coord.x)
    r = uv_pixels[pixelNumber*4 + 0]
    g = uv_pixels[pixelNumber*4 + 1]
    b = uv_pixels[pixelNumber*4 + 2]
    a = uv_pixels[pixelNumber*4 + 3]
    return (r, g, b, a)



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

    slot = obj.material_slots[f.material_index]
    material = slot.material
    image = None
    #if material is not None:
    #    image = bpy.data.textures[material.name].image

    print(bpy.data.materials[material.name].node_tree.nodes.keys())
    if not "Diffuse BSDF" in bpy.data.materials[material.name].node_tree.nodes.keys():
        continue
    principled = bpy.data.materials[material.name].node_tree.nodes["Diffuse BSDF"]
    base_color = principled.inputs[0] #Or principled.inputs[0]
    link = base_color.links[0]
    link_node = link.from_node
    print( link_node.image.name )
    image=link_node.image
    #image.filepath = "/tmp/"+image.name+".png"
    #image.save()

    for x in range(x_min_f,x_max_f+1):
        for y in range(y_min_f,y_max_f+1):
            for z in range(z_min_f,z_max_f+1):
                ptM = Vector( [x,y,z] ) #current point, in voxel frame
                vectAM = ptM - ptA
                decomp = mat_inv @ vectAM
                check = mat @ decomp
                ptT = (ptM - decomp[2]*vectW)*vox_size #closest in triangle, in blender frame
                #print("decomp: ",decomp, ptM, vectAM)
                if (decomp.x+decomp.y<1.05) and (-0.05<decomp.x) and (-0.05<decomp.y) and (-1.005<decomp.z) and (decomp.z<0.005):
                    #print("add")
                    if (add_cubes):
                        bpy.ops.mesh.primitive_cube_add(location=(x,y,z),size=1)
                    if image is not None:
                        rgba = list(getUVPixelColor(mesh, f, ptT, image))
                        coul = int(rgba[0]*31)*32*32 + int(rgba[1]*31)*32 + int(rgba[2]*31)
                    else:
                        coul = 13005000
                    all_vox.append( (x,y,z,coul) )
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
