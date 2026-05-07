# Before Effects Release 1.0.0
A cross-platform motion graphics desktop software supporting basic vector drawing and animation. Built using c++ Qt wigets and CMake.

## Features
### Selection tool
<img width="24" height="30" alt="image" src="https://github.com/user-attachments/assets/f36603ca-a21f-4b91-b03f-44d4129561d7" />
- Select any path on canvas
- Pan canvas using `middle mouse button`
- Delete the selecting path by pressing `delete`
- Scale the selected path from the 8 scale handles
- Click on a selected path to move from scale mode to rotation mode and vice versa
- Rotate the seelcted path in rotate mode from the 4 rotation handles. Paths are rotated around the pivot point, indicated by a cross shape

### Node edit tool
<img width="30" height="28" alt="image" src="https://github.com/user-attachments/assets/a70eabae-7e24-4c73-b4a9-279090929697" />
- Either select a new path or edit the selected path
- Click on a node to move it
- `shift + click` on another node to select more nodes
- `ctrl + click` on a node to change the node drawing type (linear, bezier symmetric, bezier smooth/disconnected)
- Move selected nodes by clicking and dragging

### Bezier drawing tool
<img width="30" height="30" alt="image" src="https://github.com/user-attachments/assets/26889b22-0fe4-454a-8e7e-b41d76ec6084" />
- Click to add a linear node
- Click and drag to add a bezier symmetric node. Release to set handles
- Go near the start node to see the snap rectangle. Click there to close the path and end drawing.

### Attribute panel
- A name is shown on top and is used to identify paths. Edit the content of the text edit box to change the name.
- Attributes include x/y postion, x/y scale, x/y pivot point position (relative to the initial pivot position when creating the shape), rotation, stroke width, stroke color (RGBA and HEX or use the color selector), fill color (RGBA and HEX or use the color selector), joint modes (miter, smooth, bevel joints)
<img width="213" height="224" alt="image" src="https://github.com/user-attachments/assets/baed10db-d457-46c5-8770-7aa4e5d9aff0" />
<img width="253" height="233" alt="image" src="https://github.com/user-attachments/assets/1e189f68-f06d-4923-8390-643eb7a7d5b2" />
<img width="211" height="226" alt="image" src="https://github.com/user-attachments/assets/ebb11a37-e9d5-44d3-9370-aeb62a8198cc" />


### Timeline
https://github.com/user-attachments/assets/de1e72f0-30f8-41dd-91aa-80e473c752b4
- Click and drag on the top bar to move the time indicator. Use the 5 playback control buttons to control it precisely. Use the zoom slider or buttons to zoom.
https://github.com/user-attachments/assets/6e2fe263-65f4-4c99-aa6d-69ec2d3fdeec
- Paths are shown as layers in their order of drawing and named. The selected path has a yellow highlight.
- Click on the diamond symbol on any attribute that has it in the attribute panel, a similar diamond will be shown on the corresponding layer on the current frame
<img width="158" height="34" alt="image" src="https://github.com/user-attachments/assets/615990c9-a0b6-46ee-a1b3-399f7355dc54" />
<img width="157" height="28" alt="image" src="https://github.com/user-attachments/assets/5d721ea5-f60f-42dd-bd33-8e7cacaab5bd" />


### Exporting
<img width="34" height="26" alt="image" src="https://github.com/user-attachments/assets/43de9dfc-0cf6-4ee6-bdf9-80faed9f977d" />
- export into .mp4 from the export icon on the top right corner (requires ffmpeg in your environment variables)

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



Uploading Tutorial 2.mp4…




