# Getting Started
Hey, YourLocalMoon speaking. Welcome to **PopLib**, a community version of *PopCap*'s SexyAppFramework!

## Building PopLib
### Preparing
Building **PopLib** is actually very simple. It's like any *CMake* project! Just make sure you got these installed:
- Git
- CMake (latest)
- Visual Studio 2022 (required for building!)

### The Process
Okay, first of all, clone the repository. I'll just assume you installed *Git*, so just enter these commands in your command prompt or whatever you have:

```
git clone https://github.com/teampopwork/PopLib.git --recursive
```

> The --recursive flag is crucial, it ensures all the libraries are also cloned.

After you've cloned the repository, just go into that directory, and then type this, assuming you have CMake and I won't have to explain it again:
```
cmake -S . -B build
```

**NOW,** you've got 2 paths: *Using Visual Studio to build*, and *using CMake to build*. Both still use MSVC, so just pick whatever you like. To build using Visual Studio, open the `build/` directory that was created after configuring the project, and then open the appeared `.sln` file. After Visual Studio opened, in the **Solution Explorer**, pick `CMakePredefinedTargets`, open it, right click `ALL_BUILD` and press **Build**.

To use CMake to build, just type this command in your command prompt:
```
cmake --build build --config Release
```

You can keep `Release`, OR you can replace it with `Debug` for debugging purposes. And voilà, you've built **PopLib**!

### Building with Steam API
To build with Steam API support:
1. Sign up for the Steamworks partner program
2. Download the Steamworks SDK from https://partner.steamgames.com/downloads/
3. Copy the the `sdk` folder (**FROM YOUR OWN COPY**) to the `private/steam/` directory
4. In the top-level CMakeLists.txt, set the FEATURE_STEAM_API option to ON
5. Do everything else to build

## Font Building

## Using other Tools