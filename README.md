# videoloop-by-tempo
Display video loop which changes its playback speed according to tempo of computer's audio output

## Requirements
This program has been tested to work on Ubuntu 20.04, but likely works on any modern Linux distribution. The project requires Qt for GUI and audio device handling, the <a href=https://essentia.upf.edu>Essentia library</a> for audio analysis, and a video driver which supports OpenGL for rendering. The OpenGL requirement is to enable more complex ways to display the video in the future.

## Building

### Essentia
- Create folder with `mkdir external` in the project folder
- Git clone essentia to `external` folder from https://github.com/MTG/essentia
- Install dependencies according to https://essentia.upf.edu/installing.html
    - `build-essential curl cmake libfftw3-dev libeigen3-dev`
    - On Ubuntu 20.04 at least, install `python-is-python3` to make /bin/python available
- Build Essentia's dependencies in `essentia/packaging/debian_3rdparty`:
    - FFT library by running `build_fftw3.sh`
    - Eigen3 math library for some headers `build_eigen3.sh`
- Make waf executable: `chmod +x waf`
- Configure with `./waf configure --lightweight=fftw --build-static`
- Build Essentia by running (may need sudo): `./waf` 

### videoloop-by-tempo
- Install Qt dev packages
    - `qtbase5-dev qtmultimedia5-dev`
- Build with cmake
    - `mkdir build`
    - `cd build`
    - `cmake ../`
    - `cmake --build .`


## Installation
Install Qt libraries and you're ready to go
- `libqt5widgets5 libqt5multimedia5 libqt5multimedia5-plugins`

## Usage
Run `videoloop-by-tempo` to launch the program. The working directory must be the directory which contains the `frames` directory. Command line output shows you errors and information if the frames are not found.

The video can be made fullscreen by doubleclicking it, and the fullscreen mode can be exited by doubleclicking or pressing esc key. Settings are saved automatically on exit.

By default, the program lists only audio input devices with names ending with `monitor`, since these usually monitor the audio output. If other devices such as microphone are needed, start the program with command line option `-a` or `--all-audio-inputs` to show them.

To add your own video loop, add a subfolder with sequentially named `.jpg` or `.png` images inside `frames` folder and restart the program. 

### Extracting frames from video
Frames can be extracted from a video file using http://www.ffmpeg.org/ . Example command to get frames from video `input.mp4` starting and timestamp 00:01 and ending 2 seconds later, at 24fps: `ffmpeg -i input.mp4 -ss 00:01 -t 00:02 -r 24/1 frame%04d.jpg`

Ffmpeg has many features available if you need to do more complex stuff, for example crop the video, combine them side by side, or extrapolate more frames for slow motion stuff.