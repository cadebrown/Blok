# Blok

Blok is a simple 3D block-based game (similar to Minecraft) that is meant to be extensible, performant, and fun to play!

~[Screen Shot 0](./screenshots/SS0.png)


## Building

### Linux/Unix

Currently, all dependencies of `Blok` are built & linked statically (except of course, `OpenGL`, but we use `gl3w` as a loader for that).

To build, first run `./build_libs.sh`. This should build static copies of library dependencies. 

Then, `cd build`, and `cmake ..` ensure there were no errors

Now, run `make -j4`

The executable should be located as `Blok/Blok`, so run `./Blok/Blok` to run the game!

Blok should build on any machine that has a C++11 compiler, and basic POSIX support

### Windows

(TODO: test out CMake on windows)

### MacOS

The above Linux/Unix instructions should work, but I don't have a mac to test it on...


## Engine

Eventually, I plan to port the rendering engine to a more generic interface (probably in this style of C++), that could be used for general purpose 3D rendering (not neccessarily voxel).

Eventually, I want to make that new engine compatible with Vulkan, but I'll have to learn that first

For now, OpenGL reigns supreme.


