# videoloop-by-tempo
Display video loop which changes its playback speed according to tempo of computer's audio output

## Requirements
This program has been tested to work on Ubuntu 20.04, but likely works on any modern Linux distribution. The project requires Qt for GUI and audio device handling, the <a href=https://essentia.upf.edu>Essentia library</a> for audio analysis, and a video driver which supports OpenGL for rendering. The OpenGL requirement is to enable more complex ways to display the video in the future.

## Building

### Essentia
- Git clone essentia to `external` -folder from https://github.com/MTG/essentia
- Install dependencies according to https://essentia.upf.edu/installing.html
    - `build-essential curl cmake libfftw3-dev libeigen3-dev`
    - On ubuntu 20.04, install `python-is-python3` to make /bin/python available
- Build Essentia's dependencies in `essentia/packaging/debian_3rdparty`:
    - FFT library by running `build_fftw3.sh`
    - Eigen3 math library for some headers  `build_eigen3.sh`
- Configure with `./waf configure --lightweight=fftw --build-static`
- Build with `./waf`
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
Run `videoloop-by-tempo` to launch the program. The video can be made fullscreen by doubleclicking it, and the fullscreen mode can be exited by doubleclicking or pressing esc key. Settings are saved automatically on exit.

To add your own video loop, add a subfolder with sequentially named `.jpg` or `.png` images inside `frames` -folder and restart the program.