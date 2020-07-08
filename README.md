# videoloop-by-tempo
Display video loop which changes its playback speed according to tempo of computer's audio output

## Requirements

This project requires Qt for GUI and audio device handling, the <a href=https://essentia.upf.edu>Essentia library</a> for audio analysis, and a video driver which supports Vulkan for rendering.


## Usage
Currently the program requires the video loop in format of sequentially named .jpg files in frames -folder. Only two-directional loop is currently supported, meaning that after all frames have been shown once, they will be shown in backwards order.