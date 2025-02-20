# MeshToImage
Being an attempt to load high-res meshes and turn viewing angle into one or two images containing RGB and height info.
The project stems from an idea to use detailed meshes of organic objects (such as from the Megascans project) and convert them to terrain as a means to produce decent terrain very quickly. The reason is that the other quick alternative, procedurally generated landscapes, simply look too artificial without extensively reworking the result to the point that you might as well have created it manually anyway.  

This project is written in C++ and uses OpenGL to render meshes. Because of its limited scope, both the renderer and mesh importers are very limited.
Included libraries are GLFW and GLEW to create the renderer with GLM to help. STB to load textures, the fbx sdk to load .fbx and json.hpp to parse .gltf.
The project is currently limited to importing a single mesh: no submeshes. Since my experience with mesh-files is rather limited, it is very probable that I have not taken various .fbx and .gltf structures into account. It works with those meshes I have to test with.

# How it is used
The program is executed from the command line with a mesh file and a texture file as necessary arguments. It will load a single mesh from a .obj, .fbx or .gltf file, with just a main texture. It will be presented unlit in perspective and can be zoomed and rotated using a mouse. When the user is satisfied with how the mesh is presented, it is possible to switch to orthographic mode with further camera controls mapped to the keyboard, and take a snapshot and save RGB + Z to image files. While it is possible to take snapshots in perspective view, the resulting depth data has much lesser quality.

# Controls
Left mouse button and move mouse: rotate object.
Right mouse button and move mouse: zoom in/out.
WASD to pan the camera.
Q/E to zoom in/out in orthographic mode.
Z/X to limit/extend depth of field in orthographic mode.
V to toggle wireframe mode.
Space to toggle between orthographic and perspective.
Return to take a snapshot.