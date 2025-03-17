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
> MacOS: From Mountain Lion onwards, Apple no longer provides an X server by default. Please make sure you install [XQuartz](https://www.xquartz.org/) (and by extension the Xcode Command Line Tools) before you try to compile or run usnake.
>
> Haiku: You will need to install the Xlib compatibility layer [xlibe](https://depot.haiku-os.org/#!/?bcguid=bc1-FWRS&repos=haikuports&arch=x86_64&incldev=true&onlynatv=false&viewcrttyp=ALL&srchexpr=xlibe).

It is pretty easy to compile usnake. In fact, it can be done with one command: <br>
`/usr/bin/clang -g usnake.c -o usnake -lX11` or `/usr/bin/gcc -g usnake.c -o usnake -lX11`

Just make sure to omit `-lX11` when compiling on Windows.
