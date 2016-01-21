Command line build:
mkdir build
cd build
cmake ..
make

Launch args:
engine [-debug] [script]
[-debug] uses the logging-only renderer; not very useful anymore
[script] defaults to "test.lua"