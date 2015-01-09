ldso A Utility To Load Shared Library To A Specify Process
===

Build
===
x64: make

android-arm: ndk-build

Usage
===
./ldso <your pid> <your so path>

Principle
===
1. Search every plt entries of the target process, until dlopen found. (got_finder.h/cpp)
2. Call the entry remotely. (arch/plt_caller.cpp)
