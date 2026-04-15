# PopLib
> An updated version of PopCap's SexyAppFramework, which aims to add more features than usual.

## Features
- **SDL3** rendering instead of plain old DirectDraw
- **x64** support
- **OpenAL** sound system, replacing the old DirectSound
- Replaced **zlib, libpng, libjpeg, ~~JPEG2000~~** with **stb_image**
- Platform independency instead of relying on the old Windows API.  

## Third-party libraries
> see NOTICE

## Contributing
> see CONTRIBUTING.md

## License
PopCap Games Framework License Version 1.1.
Copyright 2005-2009 PopCap Games, Inc. All rights reserved.

Modifications and additions by Team PopWork.
GNU Affero General Public License, version 3.

## Project insights
[![Build Status](https://github.com/teampopwork/PopLib/actions/workflows/build.yml/badge.svg)](https://github.com/teampopwork/PopLib/actions/workflows/build.yml) The build status is currently passing at 92%.  For Ubuntu related builds, you may not be so lucky with configuring and building because SDL_XSCRNSAVER is being deprecated in favor of wayland, as mentioned in issue #7.  We will be working on a solution to add SDL_VIDEODRIVER=wayland to ensure proper building on ubuntu. ![Repo size](https://img.shields.io/github/repo-size/teampopwork/poplib) ![Last commit](https://img.shields.io/github/last-commit/teampopwork/poplib)

## Building
> see docs/getting_started.md

---

*Copyright 2025 Team PopWork. All rights reserved.*
