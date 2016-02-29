Required libs:
(can be installed locally in /lib)
lua, GLFW3, SDL2

Command line build:
mkdir build
cd build
cmake ..
make

Launch args:
engine [-sdl] [script]
-sdl: Use SDL as window manager. Defaults to GLFW.
script: Defaults to "snake.lua".
