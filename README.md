# MeshToImage
Being an attempt to load high-res meshes and turn viewing angle into one or two images containing RGB and height info.
The project stems from an idea to use detailed meshes of organic objects (such as from the Megascans project) and convert them to terrain as a means to produce decent terrain very quickly. The reason is that the other quick alternative, procedurally generated landscapes, simply look too artificial without extensively reworking the result to the point that you might as well have created it manually anyway.  

This project is written in C++ using VS2022 and uses OpenGL to render meshes. Compilation of the .exe-files is done by VS2022 but there are no Windows dependencies so will hopefully work on different systems.

Because of its limited scope, both the renderer and mesh importers are very basic. Included libraries are GLFW and GLEW to create the renderer with GLM to help. STB to load textures, the fbx sdk to load .fbx and json.hpp to parse .gltf. The project is currently limited to importing a single mesh: no submeshes. Since my experience with mesh-files is rather limited, it is very probable that I have not taken various .fbx and .gltf structures into account. It works with those meshes I have to test with.

Noticing how the fbx sdk bloats the build, we decided to make a leaner version with only .gltf and .obj support. The resulting .exe is less than 1 mb. Both types are supplied in the repo.

Basic code by Borealian (all the bugs are belong to me!).
Refactoring by Johno (if it looks great, he did it).

## Remaining issues
Panning (in ortho-mode) changes the perspective (sounds weird but you'll see what I mean).

Low resolution texture with high resolution mesh fails to bind: mesh renders black.

Image output is still limited to raw .tga for RGB and ascii .pgm for 16-bit grayscale.

## How it is used
The program is executed from the command line with a mesh file and a texture file as necessary arguments. It will load a single mesh from a .obj, .fbx or .gltf file, with just a main texture. It will be presented unlit in perspective and can be zoomed and rotated using a mouse. When the user is satisfied with how the mesh is presented, it is possible to switch to orthographic mode with further camera controls mapped to the keyboard, and take a snapshot and save RGB + Z to image files. While it is possible to take snapshots in perspective view, the resulting depth data has much lesser quality.

From the command prompt: "meshtoimage -m path/meshfile -t path/texture" where both mesh and texture are necessary to start the program but can be in desired order (i.e. -t texture can be before -m mesh).

Additional commands: 	-s float_scale where default is 0.01 (mesh-files that are prime candidates for this program tend to be too large for the renderer).
						-f will flip the texture vertically. To be precise, the program flips by default and this command un-flips. If the texture looks broken, try this.

## Controls
Left mouse button and move mouse: rotate object.

Right mouse button and move mouse: zoom in/out.

WASD to pan the camera.

Q/E to zoom in/out in orthographic mode.

Z/X to limit/extend depth of field in orthographic mode.

V to toggle wireframe mode.

Space to toggle between orthographic and perspective.

Return to take a snapshot.

## Build it yourself?
Please note that my attempts to build an md-version (dynamic runtime libraries) made a build that only works on my machine :( so I have adopted code and libraries for static libraries (meaning glfw3_mt.lib instead of glfw3.lib, and mt-versions of the fbxsdk libs instead of the md-versions).
Links are at the bottom.

Note that the lib folder in the repo contains the necessary includes of all libraries apart from fbx. Also note that GLM, STB and json.hpp are header only libraries so should require minimal effort to include. GLFW and GLEW need to be linked but this process is well documented elsewhere, as is linking the fbx sdk (I found https://www.youtube.com/watch?v=oIKnBVP2Jgg helpful). If making a md-build (default) you need to change the pragmas relating to fbx in mesh.cpp to md-libs. You need to edit those pragmas regardless to accommodate your file paths. 

### OpenGL libs
GLEW (https://glew.sourceforge.net/)
GLFW (https://www.glfw.org/download)
GLM (https://github.com/g-truc/glm)

### Texture loading lib
STB (https://github.com/nothings/stb)

### FBX SDK
fbxsdk (https://aps.autodesk.com/developer/overview/fbx-sdk)

### JSON parser
json.hpp (https://github.com/nlohmann/json)
