# uSnake
The popular game of Snake, but instead written for as many platforms as possible. <br>
Based off of MattKC's [source code](https://mattkc.com/etc/snakeqr), this project also inherits the same [license](/LICENSE).

## Requirements for running uSnake
- Windows XP (see **Compiling for Windows XP** below)
  - otherwise Windows 7
- macOS X Snow Leopard
  - macOS X Tiger and Leopard might be possible (using `-mmacosx-version-min=10.4`) however is untested.
- Linux kernel verison 2.6.0 or greater
- Haiku (beta 5)

## Requirements for compiling uSnake
- Windows 10 *recommended* (in order to use MSYS2 MINGW64/MINGW32)
  - Compiling with Windows 8.1 and below *could be possible*, however that is up to you.
- Any version of macOS which can install GCC
  - [Homebrew](brew.sh) requires at least macOS Ventura, [MacPorts](macports.org) requires at least MacOS X Leopard
- Any **supported** version of any Linux distribution
  - It is likely possible to compile uSnake using Debian *Etch* however good luck getting `apt` working :P

---

## Compiling
> [!IMPORTANT]
> **Linux**: Make sure the development headers for X11 are installed: <br>
> Debian-based distributions: `libx11-dev` <br>
> Fedora-based distributions: `libX11-devel` <br>
> Arch-based distributions: `libx11` <br>
> 
> **MacOS**: From Mountain Lion onwards, Apple no longer provides an X server by default. Please make sure you install [XQuartz](https://www.xquartz.org/) (and by extension the Xcode Command Line Tools) before you try to compile or run usnake.
>
> **Haiku**: You will need to install the Xlib compatibility layer [xlibe](https://depot.haiku-os.org/#!/?bcguid=bc1-FWRS&repos=haikuports&arch=x86_64&incldev=true&onlynatv=false&viewcrttyp=ALL&srchexpr=xlibe).

These compilation instructions assume there is no cross-compilation (i.e. you compile a Windows executable on Windows), and that you are using the GNU Compiler Collection (GCC) <br>
(although in most cases you can compile with Clang too, in which case you can just change gcc to clang).

### Compiling for Linux (and Haiku)
It is pretty easy to compile usnake. In fact, it can be done immediately (as long as all prerequisites are met, read the note above for more info): <br>
`gcc -g usnake.c -o usnake -lX11` <br>

### Compiling for Windows 
First, make sure you have [MSYS2](https://www.msys2.org/) installed and updated. <br>
Then, make sure that the correct MinGW toolchain is installed:

`$ pacman -S mingw-w64-x86_64-toolchain` to compile for 64-bit Windows <br>
`$ pacman -S mingw-w64-i386-toolchain` to compile for 32-bit Windows

#### Compiling for Windows 7 and above
Replace '##' with either 32 or 64 depending on the target architecture: <br>
`C:\msys64\mingw##\bin\gcc -g usnake.c -o usnake-gcc-win## -lgdi32 -luser32`

#### Compiling for Windows XP
Replace '##' with either 32 or 64 depending on the target architecture: <br>
`C:\msys64\mingw##\bin\gcc -g usnake.c -o usnake-gcc-win## -lgdi32 -luser32 -D_WIN32_WINNT=0x0501`

### Compiling for macOS
For some reason, gcc on macOS fails using the standard Linux command unless you manually specify the X11 include and lib directories. <br>
Replace `path/to` with the folder in which `X11/include` and `X11/lib` can be found (usually `usr` or `opt`): <br>
`/usr/bin/gcc -g usnake.c -o usnake-gcc -lX11 -I /path/to/X11/include -L /path/to/X11/lib/`

