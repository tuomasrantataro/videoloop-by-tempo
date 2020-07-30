# videoloop-by-tempo
Display video loop which changes its playback speed according to tempo of computer's audio output

## Requirements
This program has been tested to work on Ubuntu 20.04, but likely works on any modern Linux distribution. The project requires Qt for GUI and audio device handling, the <a href=https://essentia.upf.edu>Essentia library</a> for audio analysis, and a video driver which supports OpenGL for rendering. The OpenGL requirement is to enable more complex ways to display the video in the future.

## Installation
Run the `install.sh` script as sudo to install required runtime files. What it does:
- Installs the required Qt packages using apt
- copies `libessentia.so` to /usr/lib

## Usage
Run `videoloop-by-tempo` to launch the program. The video can be made fullscreen by doubleclicking it, and the fullscreen mode can be exited by doubleclicking or pressing esc key. Settings are saved automatically on exit.

To add your own video loop add a subfolder with sequentially named `.jpg` or `.png` images inside `frames` -folder.