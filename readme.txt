Required libs:
(can be installed locally in /lib)
lua (lua.h lua.hpp lauxlib.h lualib.h luaconf.h)
GLFW3 (GLFW/glfw3.h GLFW/glfw3native.h)

Command line build:
mkdir build
cd build
cmake ..
make

Launch args:
engine [script]
[script] defaults to "snake.lua"