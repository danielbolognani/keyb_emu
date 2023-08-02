# keyb_emu
Keyboard emulator

Simulate keycodes using a shortcut

Usage:
Keyb_Emu <shortcut> <string> <time>

shortcut - F1 to F12
Currently only accepting Function keys as shortcuts

string - Text to be sent to keyboard buffer, currently the buffer will always terminate with an ENTER

time - Time between Keydown/Keyup and between keys



Cmake 3.14 or superior

Build - Windows
VS2019 or 20222

create a dir for build and:
cmake ..\Keyb_Emu\

msbuild Keyb_Emu.sln
or open it on Visual Studio and build it.



Build - Linux
Tested on GCC 8.3

create a dir for build and:
cmake ../Keyb_emu/

make
