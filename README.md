# Perfect Dark port (`port-net`)

## Experimental netplay branch

This branch of the port contains an **extremely** early and experimental implementation of network play.  
**DISCLAIMER:** This is **NOT READY** for prime time. Use at your own risk if you are not a developer.  
**Do not create issues about problems in this branch until this disclaimer is gone.**

See [this file](https://github.com/fgsfdsfgs/perfect_dark/blob/port-net/docs/netplay.md) for more information on how this works.

## Original description

This repository contains a work-in-progress port of the [Perfect Dark decompilation](https://github.com/n64decomp/perfect_dark) to modern platforms.

To run the port, you must already have a Perfect Dark ROM, specifically one of the following:
* `ntsc-final`/`US V1.1`/`US Rev 1` (md5 `e03b088b6ac9e0080440efed07c1e40f`).  
  **This is the recommended version to use**.  
  Called `NTSC version 8.7 final` on the boot screen.
* `ntsc-1.0`/`US V1.0` (md5 `7f4171b0c8d17815be37913f535e4e93`).  
  Technically supported, but not recommended.  
  Called `NTSC version 8.7 final` on the boot screen as well.
* `jpn-final` (md5 `538d2b75945eae069b29c46193e74790`).  
  Technically supported, but requires a separate custom-built executable.  
  Called `JPN version 8.9 final` on the boot screen.
* `pal-final` (md5 `d9b5cd305d228424891ce38e71bc9213`).  
  Technically supported, but requires a separate custom-built executable.  
  Called `PAL 8.7 final` on the boot screen.

## Status

The game is in a mostly functional state, with both singleplayer and split-screen multiplayer modes fully working.  
There are minor graphics- and gameplay-related issues, and possibly occasional crashes.

**The following extra features are implemented:**
* mouselook;
* dual analog controller support;
* widescreen resolution support;
* configurable field of view;
* 60 FPS support, including fixes for some framerate-related issues;
* fixes for a couple original bugs and crashes;
* basic mod support, currently enough to load a few custom levels;
* slightly expanded memory heap size;
* experimental high framerate support (up to 240 FPS):
  * set `Game.TickRateDivisor` to `0` in `pd.ini` to activate;
  * in practice the game will have issues running faster than ~165 FPS, so use VSync or `Video.FramerateLimit` to cap it.

Currently only 32-bit platforms are supported, namely x86 Windows and Linux.  
Note that 32-bit binaries will still work on 64-bit versions of those platforms,
though you might have to install some additional libraries.

## Download

Latest [automatic builds](https://github.com/fgsfdsfgs/perfect_dark/actions) of the netplay branch for supported platforms:
* [i686-windows (`port-net`)](https://nightly.link/fgsfdsfgs/perfect_dark/workflows/c-cpp/port-net/pd-i686-windows.zip)
* [i686-linux (`port-net`)](https://nightly.link/fgsfdsfgs/perfect_dark/workflows/c-cpp/port-net/pd-i686-linux.zip)

If you are looking for regular builds (the `port` branch), see [this link](https://github.com/fgsfdsfgs/perfect_dark/blob/port/README.md#download).

## Running

You must already have a Perfect Dark ROM to run the game, as specified above.  

1. Create a directory named `data` next to `pd.exe` if it's not there.
2. Put your Perfect Dark NTSC ROM named `pd.ntsc-final.z64` into it.
3. Run `pd.exe`.

If you want to use a PAL or JPN ROM instead, put them into the `data` directory and run the appropriate executable:
* PAL: ROM name `pd.pal-final.z64`, EXE name `pd.pal.exe`.
* JPN: ROM name `pd.jpn-final.z64`, EXE name `pd.jpn.exe`.

Note that users with different ROMs can't play netgames with each other.

Additional information can be found in the [wiki](https://github.com/fgsfdsfgs/perfect_dark/wiki).

A GPU supporting OpenGL 3.0 or above is required to run the port.

## Controls

1964GEPD-style and Xbox-style bindings are implemented.

N64 pad buttons X and Y (or `X_BUTTON`, `Y_BUTTON` in the code) refer to the reserved buttons `0x40` and `0x80`, which are also leveraged by 1964GEPD.

Support for one controller, two-stick configurations are enabled for 1.2.

Note that the mouse only controls player 1.

Controls can be rebound in `pd.ini`. Default control scheme is as follows:

| Action           | Keyboard and mouse     | Xbox pad                 | N64 pad                   |
| -                | -                      | -                        | -                         |
| Fire / Accept    | LMB/Space              | RT                       | Z Trigger                 |
| Aim mode         | RMB/Z                  | LT                       | R Trigger                 |
| Use / Cancel     | E                      | N/A                      | B                         |
| Use / Accept     | N/A                    | A                        | A                         |
| Crouch cycle     | N/A                    | L3                       | `0x80000000` (Extra)      |
| Half-Crouch      | Shift                  | N/A                      | `0x40000000` (Extra)      |
| Full-Crouch      | Control                | N/A                      | `0x20000000` (Extra)      |
| Reload           | R                      | X                        | X `(0x40)`                |
| Previous weapon  | Mousewheel forward     | B                        | D-Left                    |
| Next weapon      | Mousewheel back        | Y                        | Y `(0x80)`                |
| Radial menu      | Q                      | LB                       | D-Down                    |
| Alt fire mode    | F                      | RB                       | L Trigger                 |
| Alt-fire oneshot | `F + LMB` or `E + LMB` | `A + RT` or  `RB + RT`   | `A + Z`     or `L + Z`    |
| Quick-detonate   | `E + Q`   or `E + R`   | `A + B`  or  `A + X`     | `A + D-Left`or `A + X`    |

## Building

### Windows

1. Install [MSYS2](https://www.msys2.org).
2. Open the `MINGW32` prompt. (**NOTE:** _not_ the `MSYS` prompt or the `MINGW64` prompt)
3. Install dependencies:  
   `pacman -S mingw-w64-i686-toolchain mingw-w64-i686-SDL2 mingw-w64-i686-zlib make git`
4. Get the source code:  
   `git clone -b port-net --recursive https://github.com/fgsfdsfgs/perfect_dark.git && cd perfect_dark`
5. Run `make -f Makefile.port`.
   * Add ` ROMID=pal-final` or ` ROMID=jpn-final` at the end of the command if you want to build a PAL or JPN executable respectively.
6. The resulting executable will be at `build/ntsc-final-port/pd.exe`.
7. If you don't know where you downloaded the source to, you can run `explorer .` to open the current directory.

### Linux

1. Ensure you have gcc, g++ (version 10.0+) and 32-bit versions of SDL2 (version 2.0.12+), libGL and ZLib installed on your system.
   * On a 64-bit system you also need to have `gcc-multilib` and `g++-multilib` (or your distro's analogues) installed.
2. Get the source code:  
   `git clone -b port-net --recursive https://github.com/fgsfdsfgs/perfect_dark.git && cd perfect_dark`
3. Run the following command:
   * On a 64-bit system: ```make -f Makefile.port TARGET_PLATFORM=i686-linux```
   * On a 32-bit system: ```make -f Makefile.port```
   * Add ` ROMID=pal-final` or ` ROMID=jpn-final` at the end of the command if you want to build a PAL or JPN executable respectively.
4. The resulting executable will be at `build/ntsc-final-port/pd.exe`.  

Currently only `i686-linux` and `i686-windows` are supported, using `gcc -m32` and `i686-w64-mingw32-gcc` as compilers, respectively.  
Alternate compilers can be specified by passing `TOOLCHAIN=i686-whatever-` as a command line argument.

You can build an executable with PAL or JPN ROM support by adding `ROMID=pal-final` or `ROMID=jpn-final` to the `make` command.  
You will need to provide a `jpn-final` or `pal-final` ROM to run those, named `pd.jpn-final.z64` or `pd.pal-final.z64`.

It might be possible to build a 32-bit ARM executable, but this has not been tested.

## Credits

* the original [decompilation project](https://github.com/n64decomp/perfect_dark) authors;
* doomhack for the only other publicly available [PD porting effort](https://github.com/doomhack/perfect_dark) I could find;
* [sm64-port](https://github.com/sm64-port/sm64-port) authors for the audio mixer and some other changes;
* [Ship of Harkinian team](https://github.com/Kenix3/libultraship/tree/main/src/graphic/Fast3D), Emill and MaikelChan for the libultraship version of fast3d that this port uses;
* lieff for [minimp3](https://github.com/lieff/minimp3);
* Mouse Injector and 1964GEPD authors for some of the 60FPS- and mouselook-related fixes;
* everyone who has submitted pull requests and issues to this repository and tested the port;
* probably more I'm forgetting.
