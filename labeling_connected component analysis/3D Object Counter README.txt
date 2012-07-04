http://www.nabble.com/Object-Counter-3D-to13547064.html

by Fabrice Cordelières Nov 02, 2007; 10:51pm 

Dear Thomas,

As I am the one who "committed" this plugin, I will briefly answer your
questioning.
3D object counter is grouping object pixels into particles by looking at one
pixel's 8 neighbors in 2D and 26 neighbors in 3D. Doing so it will label all
particles and return as "volume" the number of pixels/voxels forming each
particle, and as "surface" the number of pixels/voxels lacking at least one
of its neighbors. The way one pixel is "lacking neighbors" might vary in
many ways (for example, one surface along xy plane or one surface along xz
plane may not result in the same surface, as you've mentioned).  This is
obviously not a true measure of the surface therefore I will change the
denomination of this line within the table of results so that no one could
be mistaken. I'm currently working on an updated version of the plugin and
will take your comment into account.

Fabrice
 
...
Particularly, I was wondering if you had considered implementing a  
Hoshen-Kopelman algorithm for determining 3D connectivity of particles  
and the resulting 'true' surface area of each particle.
