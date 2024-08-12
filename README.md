x265_encoder for Davinci Resolve Studio (based on Black Magic's x264_encoder_plugin sample)

[WINDOWS]

To compile this under WINDOWS, you'll need the following tools installed

1) Visual Studio 2022
2) MSYS2

Instructions:

[Pre-Req]

Create Directory C:\VideoEditingUtils\x265_plugin_build

[Download x265]

1) run "MSYS2 MSYS" - type it in the search bar in the Windows Task Bar
2) pacman -Sy pacman
3) pacman -Syu
4) pacman -Su
5) pacman -S nasm
6) pacman -S make
7) pacman -S git
8) cd /C/VideoEditingUtils/x265_plugin_build   
90) git clone https://github.com/videolan/x265

[Compile x265]

1) MODIFY the file "C:\VideoEditingUtils\x265_plugin_build\x265\build\msys-cl\make-Makefiles-64bit.sh"

> in line number 5, change "target_processor='amd64'" to "target_processor='x64'"

> in line 24, append the following defintions -DENABLE_SHARED=OFF -DENABLE_CLI=OFF -DSTATIC_LINK_CRT=ON

Note: make sure there's a white space between these defintions and ../source

> comment out line 16-21

2) run the "x64 Native Tools Command Prompt for VS 2022" - type it in the seach bar in the Windows Task Bar
3) Inside the command prompt, type "cd /d C:\msys64"
4) Execute the command "msys2_shell.cmd -mingw64 -full-path"
5) This will open up another terminal, type in the following commands (in the new terminal)
6) cd /C/VideoEditingUtils/x265_plugin_build/x265/build/msys-cl
7) "./make-Makefiles-64bit.sh"

[Download x265_encoder]

1) run "MSYS2 MSYS" - type it in the search bar in the Windows Task Bar
2) cd /C/VideoEditingUtils/x265_plugin_build
3) git clone https://github.com/UDaManFunks/x265_encoder

[Compile x265_encoder]

1) run the "x64 Native Tools Command Prompt for VS 2022" - type it in the seach bar in the Windows Task Bar
2) Inside the command prompt, type "cd /d C:\VideoEditingUtils\x265_plugin_build\x265_encoder"
3) nmake /f NMakefile
   
[Packaging / Installing]

1) Create folder called "IOPlugins" under %PROGRAMDATA%\Blackmagic Design\DaVinci Resolve\Support

  Note: you can open up %PROGRAMDATA% folder via explorer by typing it verbatim in a run window (Win + R) 

2) Create a folder named "x265_encoder.dvcp.bundle" under the IOPlugins folder
3) Create a folder named "Contents" under the "x265_encoder.dvcp.bundle" folder
4) Create a folder named "Win64" under the "Contents" folder
5) Copy the built plugin from C:\VideoEditingUtils\x265_plugin_build\x265_encoder\bin\x265_encoder.dvcp" and place it in the "Win64" folder (which you've created via Step 1-4)
6) Start Davinci Resolve Studio
   
You can export using X265 if you pick "QUICKTIME" or "MP4" as your FORMAT in Davinci Resolve Studio, then selecting the "X265 (8-bit)" Codec option.

----------------- [LINUX] -----------------

To compile this under LINUX, you'll need certain development tools installed.   The commands listed below were tested in UBUNTU 24.04 LTS


Instructions:

[Pre-Req]

Create a directory in your HOME directory named x265_plugin_build

> mkdir ~/x265_plugin_build

Make sure the following package is installed 

> apt-get install cmake-curses-gui

[Download x265]

> cd ~/x265_plugin_build

> git clone https://github.com/videolan/x265

[Compile x265]

> cd  ~/x265_plugin_build/x265/build/linux

> ./make-Makefiles.sh

SELECT or CHANGE th following options

> CMAKE_INSTALL_PREFIX = ../../../x265_pkg

> ENABLE_CLI = OFF

> ENABLE_SHARED = OFF

> ENABLE_PIC = ON

> STATIC_LINK_CRT = OFF

> hit the letter "c" to configure

Note: 

Compile and install include / binaries

> make -j 8

> make install

[Download x265_encoder]

> cd ~/x265_plugin_build

> git clone https://github.com/UDaManFunks/x265_encoder

[Compile x265_encoder]

> cd ~/x265_plugin_build/x265_encoder

> make
   
[Packaging / Installing]

Create the plugin folder structure

> sudo mkdir -p /opt/resolve/IOPlugins/x265_encoder.dvcp.bundle/Contents/Linux-x86-64

Move the newly built binary the to the folder you created

> sudo mv bin/x265_encoder.dvcp /opt/resolve/IOPlugins/x265_encoder.dvcp.bundle/Contents/Linux-x86-64/

> chown root:root /opt/resolve/IOPlugins/x265_encoder.dvcp.bundle/Contents/Linux-x86-64/x265_encoder.dvcp

> chmod 755 /opt/resolve/IOPlugins/x265_encoder.dvcp.bundle/Contents/Linux-x86-64/x265_encoder.dvcp

Restart Davinci Resolve Studio 
