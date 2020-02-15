# Blok

Blok is a simple 3D block-based game (similar to Minecraft) that is meant to be extensible, performant, and fun to play!

## Building

Currently, all dependencies of `Blok` are built & linked statically (except of course, `OpenGL`, but we use `gl3w` as a loader for that).

To build, first run `./build_libs.sh`. This should build static copies of library dependencies. 

Then, `cd build`, and `cmake ..` ensure there were no errors

Now, run `make -j4`

The executable should be located as `Blok/Blok`, so run `./Blok/Blok` to run the game!

Blok should build on any machine that has a C++11 compiler, and basic POSIX support



