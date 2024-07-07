# RayTileMap
A simple library to read and draw tilemaps from common tilemap editors, such as Tiled.

## Work In Progress
Currenlty supports basic orthographic TMX files that are not compressed.

### TXM export settings
Be sure to save your TMX as Base64Uncompressed or CSV

# API
Main API is in ray_tilemap.h

Create a TileMap variable for your game to use.
Call LoadTileMap with the TMX file path and pass in your map.

Draw the tilemap with DrawTileMap, and pass in an optional camera and view bounds.

if you want to insert your own drawing code between two layers, insert a user/virtual layer with InsertTileMapLayer<VirtualLayer>

The virtual layer has a callback that you can install that will be called when the layer should be drawn.

You can add drawables to any tile layer that will be drawn on each Y row is processed.

# Building
Add the following cpp files to your build (or make a lib out of them)

ray_tilemap.cpp
ray_tilemap_drawing.cpp
ray_tilemap_tmx.cpp
include/external/PUGIXML/pugixml.cpp

# TODO
* Layer Drawables (Alpha)
* Object Layers (Alpha)
* Collision API
* Draw Order
* Tilemap Compression
* Isometric Support

# License
Copyright (c) 2020-2024 Jeffery Myers

This software is provided "as-is", without any express or implied warranty. In no event 
will the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial 
applications, and to alter it and redistribute it freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not claim that you 
  wrote the original software. If you use this software in a product, an acknowledgment 
  in the product documentation would be appreciated but is not required.

  2. Altered source versions must be plainly marked as such, and must not be misrepresented
  as being the original software.

  3. This notice may not be removed or altered from any source distribution.
