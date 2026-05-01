# Before Effects Release 1.0.0
A cross-platform motion graphics desktop software supporting basic vector drawing and animation. Built using c++ Qt wigets and CMake.

## Features
1. Bezier drawing (click to add a linear node, click and drag to add a symmetric node)
2. shape node editing (move nodes, move handles, switche betweel handle types using `ctrl + click`)
3. adding and deleting paths
4. moving, rotating, scaling
5. panning (middle mouse button)
6. keyframing
7. linear interpolation animation
8. Timeline controls (move to the start/end, move one frame forward/backward, play/pause)


9. **Attribute panel:** position, scale, pivot point, rotation, fill color, stroke color, stroke width, joint types
10. exporting into .mp4 (requires ffmpeg in your environment variables)

## Planned features
- viewport controls
- importing SVGs
- exporting to a file
- exporting to an mp4
- Ctrl + C, V, Z, Y
- keyframe interpolation and graph editor
- Text object
- Text object properties
- Image object
- Image object properties
- importing other formats
- complex SVG shapes
- path manipulation and masking

## Download
Download the zip file from the [release page](https://github.com/AdamLotfalla/Before-Effects/releases/tag/v1.0.0). Just un-zip it and run.
### ffmpeg (for exporting)
- For Windows
```
winget install ffmpeg
```
- For Mac
```
brew install ffmpeg
```
- For Linux: depends on your system's package manager

Ubuntu, Debian, Linux Mint, and Kali
``` 
sudo apt update && sudo apt install ffmpeg -y
```
Fedora
``` 
sudo dnf install ffmpeg
```



https://github.com/user-attachments/assets/a35811cd-c3b5-432d-9893-35b01095e242


https://github.com/user-attachments/assets/08c116c1-bcec-41bb-8c6e-2b262f1ce727
