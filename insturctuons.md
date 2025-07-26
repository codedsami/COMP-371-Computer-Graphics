Programming Project Assignment 1
Due: First day of week 5. (Week of July 28.)

Implementation specifications:
Only the libraries used in the course tutorials may be used, unless otherwise specified. This means:
OpenGL
GLFW
GLM
GLEW (or GLAD)
stb_image.h
OBJloader from Tut05 (or Assimp)
The application must feature an interactive camera controlled with mouse and keyboard. The camera must be able to:
Rotate to change its viewing direction.
Translate around the scene, in a “flying” or “walking” fashion.
The movement controls that result in camera translation must be relative to the camera facing direction. (i.e. pressing “forward” makes the camera go forward relative to the direction it is currently facing).
The 3D scene must feature multiple textured surfaces with different textures.
The 3D scene must feature at least one of the following:
Dynamic lighting with multiple moving light sources. (Meaning the lights move around the scene).
Elements of the scene that are animated hierarchically with rotation, at least 2 levels deep. (This means something such as an articulated robot arm, or an orrery with orbiting planets and moons.)
The texture images and 3D scene should be visually different than the ones in the tutorials.