# Before Effects Release 1.1.0
A cross-platform motion graphics desktop software supporting basic vector drawing and animation. Built using c++ Qt wigets and CMake.

https://github.com/user-attachments/assets/6e815b78-18b6-4250-96a5-9d9c6fbae0cd


## Features

Use the follwing keys to raise/lower paths over/behind others:
- `Page up`: raise by one step
- `Page down`: lower by one step
- `End`: lower to bottom
- `Home`: raise to top

### <img width="18" height="22.5" alt="image" src="https://github.com/user-attachments/assets/f36603ca-a21f-4b91-b03f-44d4129561d7" /> Selection tool (Shortcut: S)


- Select any path on canvas
- Pan canvas using `middle mouse button`
- Delete the selecting path by pressing `delete`
- Scale the selected path from the 8 scale handles
- Click on a selected path to move from scale mode to rotation mode and vice versa
- Rotate the seelcted path in rotate mode from the 4 rotation handles. Paths are rotated around the pivot point, indicated by a cross shape

### <img width="22.5" height="21" alt="image" src="https://github.com/user-attachments/assets/a70eabae-7e24-4c73-b4a9-279090929697" /> Node edit tool (Shortcut: N)


- Either select a new path or edit the selected path
- Click on a node to move it
- `shift + click` on another node to select more nodes
- `ctrl + click` on a node to change the node drawing type (linear, bezier symmetric, bezier smooth/disconnected)
- If a node is bezier, you could drage the bezier handles to edit the path
- Move selected nodes by clicking and dragging

### <img width="22.5" height="22.5" alt="image" src="https://github.com/user-attachments/assets/26889b22-0fe4-454a-8e7e-b41d76ec6084" /> Bezier drawing tool (Shortcut: B)


- Click to add a linear node
- Click and drag to add a bezier symmetric node. Release to set handles
- Go near the start node to see the snap rectangle; click there to close the path and end drawing. Or Press enter mid-drawing to finish it

### Attribute panel
- Modify attributes like
    - Name
    - Position (The top left corner is (0,0))
    - Scale
    - Rotation
    - Pivot Point (used for rotation and scale)
    - Fill Color (RBGA + HEX + color selector)
    - Enable/Disable Fill
    - Stroke Color (RBGA + HEX + color selector)
    - Enable/Disable Stroke
    - Stroke Width
    - Joint Style (Miter, Bevel, Round)

        <img width="200" height="200" alt="Bevel Joint" src="https://github.com/user-attachments/assets/3afa1ca5-ac16-4199-a262-5a80bf07c778" />
        <img width="200" height="200" alt="Round Joint" src="https://github.com/user-attachments/assets/4513e704-71bb-4793-ad5e-62cc15bf0c42" />
        <img width="200" height="200" alt="Miterl Joint" src="https://github.com/user-attachments/assets/e084d8c0-5d58-4644-946b-067f34f80d0f" />


    - Cap Style (Square, Flat, Round)



### Timeline


[https://github.com/user-attachments/assets/cc516a98-f381-4e43-bc20-877ee9542848](https://github.com/user-attachments/assets/a67d0935-b265-41f6-af6a-9f66e8041f94)


- Click and drag on the top bar to move the time indicator. Use the 5 playback control buttons to control it precisely. Use the zoom slider or buttons to zoom.
- Paths are shown as layers in their order of drawing and named. The selected path has a yellow highlight.
- Double click the layer name to rename the path.
- Show/Hide the path using the eye icon.
- Click on the diamond symbol on any attribute that has it in the attribute panel, a similar diamond will be shown on the corresponding layer on the current frame

    <img width="160" height="30" alt="Not Keyframed" src="https://github.com/user-attachments/assets/cdc053eb-826f-485e-ad8c-4e358cc274c0" />
    <img width="160" height="30" alt="Kyeframed" src="https://github.com/user-attachments/assets/95e807fe-5157-4712-999b-152a0c26bc5f" />


- Expand a layer with keyframes to see which keyframes are for which attributes.
- Drage keyframes. Select multiple by making a selection box or by pressing shift while clicking.
- Drag the boundary handles to only confine timeline to a specific range.
- Drag Layers to rearrange their order



### Exporting
<img width="34" height="26" alt="image" src="https://github.com/user-attachments/assets/43de9dfc-0cf6-4ee6-bdf9-80faed9f977d" />
- export into .mp4 from the export icon on the top right corner (requires ffmpeg in your environment variables)

## Planned features
- viewport controls
- importing SVGs
- exporting to a file
- Ctrl + C, V, Z, Y
- keyframe interpolation and graph editor
- Text object
- Image object
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
