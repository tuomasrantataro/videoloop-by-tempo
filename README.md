# videoloop-by-tempo
Display video loop which changes its playback speed according to tempo of computer's audio output

## Requirements
This program has been tested to work on Ubuntu 20.04, but likely works on any modern Linux distribution. The project requires Qt for GUI and audio device handling, the <a href=https://essentia.upf.edu>Essentia library</a> for audio analysis, and a video driver which supports OpenGL for rendering. The OpenGL requirement is to enable more complex ways to display the video in the future.

## Building
The build is automated with CMake. This includes cloning Essentia's git repo and building it.

### Dependencies
Install development packages with `apt` or some other package manager: `xargs sudo apt install -y <packages.list`
- General tools:
    - `build-essential curl cmake`
- For Qt:
    - `qtbase5-dev qtmultimedia5-dev`
- For PulseAudio:
    - `libpulse-dev`
- For Essentia:
    - `libfftw3-dev libeigen3-dev`
    - Essentia's build system needs an older python version (works with 3.7) runnable with the `python` command. Install that version and set `/usr/bin/python` to point to it with `update-alternatives`

Build with cmake
- `mkdir build`
- `cd build`
- `cmake ../`
- `cmake --build .`


## Install runtime packages
Install Qt libraries and you're ready to go
- `libqt5widgets5 libqt5multimedia5 libqt5multimedia5-plugins libqt5sql5-sqlite`

## Usage
Run `videoloop-by-tempo` to launch the program. The working directory must be the directory which contains the `frames` directory. Command line output shows you errors and information if the frames are not found.

The video can be made fullscreen by doubleclicking it, and the fullscreen mode can be exited by doubleclicking or pressing esc key. Settings are saved automatically on exit.

By default, the program lists only audio input devices with names ending with `monitor`, since these usually monitor the audio output. If other devices such as microphone are needed, start the program with command line option `-a` or `--all-audio-inputs` to show them.

By default, if listening music with Spotify, the program saves song tempo data to SQLite database `tracktempos.db` and uses the recorded tempos when playing the same track next time. This is almost always more accurate than the default mode which calulates tempo based on last 7 seconds of audio data. To disable saving tempo data, use command line option `-n` or `--no-track-logging`. To disable saved tempo use, rename or delete the database and use the `-n` option.

To add your own video loop, add a subfolder with sequentially named `.jpg` or `.png` images inside `frames` folder and restart the program. 

### Extracting frames from video
Frames can be extracted from a video file using http://www.ffmpeg.org/ . Example command to get frames from video `input.mp4` starting and timestamp 00:01 and ending 2 seconds later, at 24fps: `ffmpeg -i input.mp4 -ss 00:01 -t 00:02 -r 24/1 frame%04d.jpg`

FFmpeg has many features available if you need to do more complex stuff, for example crop the video, combine them side by side, or extrapolate more frames for slow motion stuff.
