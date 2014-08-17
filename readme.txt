Y A R S
 - Yet Another Rendering Sandbox -

--

YARS is a simple, dirty, unfinished rendering sandbox I developed while working on my master thesis.
In its current status, it shouldn't probably used by anyone, except maybe some OpenGL beginner (as I consider myself).

YARS is written in C++11 and OpenGL 4.3 with no interest for backward compatibility, aiming at CG experimentation.

--

FEATURES

YARS allow you to setup different scenes and renderers, and to select them at runtime (useful to compare different
techniques and to profile them).

Scenes are made of asset/camera/light nodes, and can be built interactively by adding nodes and setting up their
properties through the GUI. Unfortunately, the GUI is currently quite confusing, because it was mainly
built day after day by adding only what I needed and in the fastest way I could find. 

Renderers are to be coded in C++/GLSL. Adding a new Renderer should be easily done by extending the Renderer and
ShaderProgram classes.

Currently, a basic forward renderer and two more advanced deferred renderers implementing the ambient occlusion
technique developed during my master thesis work are included.
Such implementations have some little problems and restrictions here and there, but should be useful as examples.

Actually, there's no culling done on the (naive) scene graph: everything in the current Scene, as seen by the
currently selected CameraNode, gets rendered by the pipeline implemented by the current Renderer.

Some additional information is available in my thesis.

-- 

BUILD INSTRUCTIONS

The provided VS2013 solution file expects some libraries in an "external" directory at the same level of the "yars"
directory:

- Assimp 3: asset loading in a variety of formats
- glew 1.10: OpenGL extension handling
- glfw 2.7.9: windowing/input handling
- DevIL 1.7.8: image files handling
- rapidjson: scene files persistence
- AntTweakBar: GUI

 All the used libraries should be portable (Win32/Linux/Mac) but I only built YARS on Windows, using VS2013.
 Contact me if you have troubles building or running YARS.

--

UPDATE LOG

2014-08-17 update:
Unfortunately, I keep being unable to properly refactor the code as I expected, being too busy coding for paid
jobs that deserve higher priority. I did some cleaning up here and then in the last few months, but I don't 
have the necessary continous amount of time that would be necessary to do the bigger structural changes I drafted 
in my mind. So, I decided to publish the code as-is and hopefully fix it in the future

2014-02-06 status:
code cleaning in progress, will be uploaded soon

--

thanks for reading,

Dario

