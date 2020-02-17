# Blok

Blok is a simple 3D block-based game (similar to Minecraft) that is meant to be extensible, performant, and fun to play!

## Building

Currently, all dependencies of `Blok` are built & linked statically (except of course, `OpenGL`, but we use `gl3w` as a loader for that).

To build, first run `./build_libs.sh`. This should build static copies of library dependencies. 

Then, `cd build`, and `cmake ..` ensure there were no errors

Now, run `make -j4`

The executable should be located as `Blok/Blok`, so run `./Blok/Blok` to run the game!

Blok should build on any machine that has a C++11 compiler, and basic POSIX support


## Engine

Eventually, I plan to port the rendering engine to a more generic interface (probably in this style of C++), that could be used for general purpose 3D rendering (not neccessarily voxel).

Eventually, I want to make that new engine compatible with Vulkan, but I'll have to learn that first


