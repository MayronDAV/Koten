# Koten

[![License](https://img.shields.io/github/license/MayronDAV/Koten.svg)](https://github.com/MayronDAV/Koten/blob/master/LICENSE)
[![image](https://github.com/MayronDAV/Koten/workflows/linux/badge.svg)](https://github.com/MayronDAV/Koten/actions?query=workflow%3Alinux)
[![image](https://github.com/MayronDAV/Koten/workflows/windows/badge.svg)](https://github.com/MayronDAV/Koten/actions?query=workflow%3Awindows)

## Getting Started

<ins>**1. Downloading the repository:**</ins>

Start by cloning the repository with `git clone --recursive https://github.com/MayronDAV/Koten`.

If the repository was cloned non-recursively previously, use `git submodule update --recursive --init` to clone the necessary submodules.

<ins>**2. Configuring the dependencies:**</ins>

* Windows: 
    1. Run the [GenVS22.bat](https://github.com/MayronDAV/Koten/blob/main/Scripts/Windows/GenVS22.bat) file found in `Scripts\Windows` folder.
    
* Linux:
    1. Installing dependencies:

        $ sudo apt update

        $ sudo apt upgrade -y

        $ sudo apt install -y build-essential g++-11 gcc-11 libgl1-mesa-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libxxf86vm-dev mesa-common-dev libgtk-3-dev mono-complete

<ins>**3. Building:**</ins>

* Windows:
    1. Run the [Build.bat](https://github.com/MayronDAV/Koten/blob/main/Build.bat) file found in the root dir or open Koten.sln.

* Linux:
    1. Give permission to [Build-Linux.sh](https://github.com/MayronDAV/Koten/blob/main/Build-Linux.sh):

        $ chmod u+r+x Build-Linux.sh

    2. Run the [Build-Linux.sh](https://github.com/MayronDAV/Koten/blob/main/Build-Linux.sh).

***