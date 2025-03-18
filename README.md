# usnake
The popular game of Snake, but instead written for as many platforms as possible. <br>
Based off of MattKC's [source code](https://mattkc.com/etc/snakeqr), this project also inherits the same [license](/LICENSE).

## Minimum requirements
- Microsoft Windows XP (via `gcc` only)
  - `clang` requires at least Windows 7.
- Mac OS X Tiger (using the parameter `-mmacosx-version-min=10.4`)
  - Without the parameter, you will need at least Snow Leopard
- Linux kernel verison 2.6.0 or greater
- Haiku (beta 5)

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

It is pretty easy to compile usnake. In fact, it can be done with one command: <br>
`/usr/bin/clang -g usnake.c -o usnake -lX11` or `/usr/bin/gcc -g usnake.c -o usnake -lX11` <br>
<sup>In macOS, `/usr/bin/gcc` and `/usr/bin/clang` are both the same hard link. For proper GCC compilation, scroll down a bit further.</sup>

Just make sure to omit `-lX11` when compiling on Windows.

### Compiling using GCC in macOS

For macOS, since `/usr/bin/gcc` and `/usr/bin/clang` are the same thing, compiling with GCC will require you to install GCC using a package manager of your choice: <br>
Replace the hashtags with the major version number of GCC you want to install (i.e. `brew install gcc@10` for GCC 10):
- `brew install gcc@##` or `fink install gcc##` or `sudo port install gcc##` <br>

Once done, locate where `gcc` is installed (Homebrew users should find GCC in `/usr/local/Cellar/gcc/<version>/bin`) and then you can run the following command: <br>
`/usr/local/Cellar/gcc/14.2.0_1/bin/gcc-14* -g usnake.c -o usnake-gcc -lX11 -I /opt/X11/include -L /opt/X11/lib/`
