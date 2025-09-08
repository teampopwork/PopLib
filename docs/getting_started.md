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

## Font Building
This may be changed in the future due to dev wants and needs.  
Navigate to the tools/old folder.  There is a series of programs called "Fontbuilder" and "FontTester", both are executables.  To build a font, first select the font to use after launching the program.  You can also limit the amount of characters as you wish (this is especially useful for scoring, as you can do "0123456789" as a base, and the library's useful functions will automatically calculate the score with the string functions as needed). 
Once finished, hit "build".  NOW: You will notice two files.  A txt file, and a PNG file that lists all the characters you used.  **DO NOT USE THE IMAGES to draw your fonts!!! And DO NOT use those images as a resource in your resource.xml file.  Use the txt file instead.**
This is referred to as a FONT DESCRIPTOR TXT FILE.  It is a plain text file with specific functionality tailored to the font's characteristics.  Using this will help you draw the font.  Not the image!  It was popcap's older way of rendering fonts with the imagefont class.  It is convienient.

**For your font tester executable:**
Navigate to the same tools/old folder.  Click on FontTester.  You should see a similar interface, but you will notice there is a black background.  Once you import the font descriptor txt file, it will display the font in full detail.  You can colorize the font, and change its properties such as Kerning values, line spacing offsets, etc.
## Using other Tools
PopPak: This tool is used to generate main.pak files which were used in later PopCap games.  
