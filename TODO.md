# TODO List
All code tasks are listed here or should be listed here

## Features
- [X] Add physics support (use Box2D Physics).
- [X] Add UUIDs.
- [X] Add Stop (with reset) buttons to the Scene View Panel.
- [X] Add Pause functionality and the Pause button to the Scene View Panel.
- [X] Add Simulate functionality and the Simulate button to the Scene View Panel.
- [X] Add UUIDs to scenes.
- [X] Add a way to draw circles and lines.
- [X] Add scripts support (use Lua and/or C#).
- [X] Add a way to use multiple scenes.
- [X] Add a way to create render targets and use them in cameras and/or elsewhere.
- [X] Add a way to use multiple cameras with different render targets (primary is only one).
- [X] Add a game view panel.
- [X] Add a way to save and load textures.
- [ ] Add a way to create and load shader packs (instead of taking the shader files and the cache).
- [ ] Add audio support.
- [ ] Add support for the game interface.
- [ ] Add a way to change the imgui theme.

## Bugs
- [X] Fix rotation of entities in box2D.
- [X] Fix the size of widgets in the Scene View Panel when the window is resized.
- [/] Fix imgui flickering when window is resized (doesn't always happen).
- [X] Fix content browser, files on final line are being cut off.
- [ ] Fix shortcuts that should only be done once at a time.