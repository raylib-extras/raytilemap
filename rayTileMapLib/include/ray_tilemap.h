/**********************************************************************************************
*
*   RayTileMap
*
*   LICENSE: MIT
*
*   Copyright (c) 2024 Jeffery Myers
*
*   Permission is hereby granted, free of charge, to any person obtaining a copy
*   of this software and associated documentation files (the "Software"), to deal
*   in the Software without restriction, including without limitation the rights
*   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*   copies of the Software, and to permit persons to whom the Software is
*   furnished to do so, subject to the following conditions:
*
*   The above copyright notice and this permission notice shall be included in all
*   copies or substantial portions of the Software.
*
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*   SOFTWARE.
*
**********************************************************************************************/

#pragma once

#include "raylib.h"
#include "raymath.h"

#include <vector>
#include <string>
#include <map>
#include <functional>
#include <memory>

namespace RayTiled
{
    // defines a texture and it's associated tile rects that are used by a tilemap
    struct TileSheet
    {
        Texture2D Texture = { 0 };		// the texture record
        uint16_t StartingTileId = 0;	// the tile id that this sheet starts at
        std::vector<Rectangle> Tiles;	// the list of source rectangles for each tile

        inline bool HasId(uint16_t id) const
        {
            return id >= StartingTileId && id < StartingTileId + Tiles.size();
        }

        void DrawTile(uint16_t id, Rectangle destinationRectangle, uint8_t flags = 0, Color tint = WHITE) const;
    };

    // The different types of layers
    enum class TileLayerType
    {
        Tile,					// a layer that has tiles
        Object,					// a layer that has objects (shapes used for collision)
        User,				// a virtual layer provided by the game to draw dynamic things, such as players, monsters or NPCs
    };

    // Common info for all layers
    struct LayerInfo
    {
        std::string Name;                           // the layer name
        int LayerId = 0;							// an ID for the layer, not necessarily the draw order
        TileLayerType Type = TileLayerType::Tile;	// The layer type
        bool Visible = true;						// should the layer be drawn?
        bool CheckForCollisions = false;			// should the layer be used for collision checks?

        virtual ~LayerInfo() = default;
    };

    // flags for the various tile states
    constexpr uint8_t TileFlagsNone = 0;
    constexpr uint8_t TileFlagsFlipVertical = 0x02;		// tile is flipped in Y
    constexpr uint8_t TileFlagsFlipHorizontal = 0x04;	// tile is flipped in X
    constexpr uint8_t TileFlagsFlipDiagonal = 0x08;		// tile is flipped diagonally (rotated)

    // data stored for each tile instance in a tile grid
    struct TileInfo
    {
        uint16_t TileIndex = 0;		// what tile to use
        uint8_t	TileFlags = 0;		// flip flags
        uint8_t Padding = 0;		// pad to make the structure align with 4 bytes
    };

    // A layer made up of tile elements
    struct TileLayer : public LayerInfo
    {
        Vector2 Bounds = { 0 };						// the grid size of the layer
        Vector2 TileSize = { 0 };					// the size of one tile element in the grid
        std::vector<TileInfo> TileData;				// the actual tile instances

        void* UserData = nullptr;

        TileLayer() { Type = TileLayerType::Tile; }

        const TileInfo* GetTile(int x, int y, Rectangle& screenRect) const;

        // TODO Collisions

        bool CellHasTile(int x, int y, uint16_t* result = nullptr);
// 
//         bool CheckCollisionRectangle(const Rectangle& rect);
//         bool CheckCollisionCircle(const Vector2& position, float radius);

        // TODO, custom drawable

        struct Drawable
        {
            virtual ~Drawable() = default;
            virtual float GetY() = 0;
            void* UserData = nullptr;
        };

        using DawableFunction = std::function<void(TileLayer& layer, Drawable& drawable, float startX, float endX)>;

        DawableFunction CustomDrawalbeFunction = nullptr;

        std::vector<Drawable*> Drawables;

        void AddDrawable(Drawable* item);
        void RemoveDrawable(Drawable* item);
    };

    // callback used for custom layer drawing

    // a layer made of objects (shapes)
    struct ObjectLayer : public LayerInfo
    {
        ObjectLayer() { Type = TileLayerType::Object; }

        using ObjectLayerDrawFunction = std::function<void(ObjectLayer& layer, Camera2D* camera, Vector2 bounds)>;

        ObjectLayerDrawFunction DrawFunc = nullptr;

        // Object data
        enum class ObjectType
        {
            Generic,
            Ellipse,
            Point,
            Polygon,
            Text,
        };

        struct Object
        {
            int32_t Id = 0;
            ObjectType Type = ObjectType::Generic;
            Rectangle Bounds = { 0 };
            float Rotation = 0;
            bool Visible = false;
            int32_t TileID = 0;

            std::string Name;
            std::string ClassName;
            std::string TemplateName;
        };

        struct PolygonObject : public Object
        {
            std::vector<Vector2> Points;
            PolygonObject() { Type = ObjectType::Polygon; }
        };

        struct TextObject : public Object
        {
            std::string Text;
            float FontSize = 10;
            TextObject() { Type = ObjectType::Text; }
        };

        std::vector<std::unique_ptr<Object>> Objets;

        // Collision functions TBD
    };

    // a layer provided by the game
    struct UserLayer : public LayerInfo
    {
        UserLayer() { Type = TileLayerType::User; }
        void* UserData = nullptr;

        // callback used to draw custom items
        using UserLayerDrawFunction = std::function<void(UserLayer& layer, Camera2D* camera, Vector2 bounds)>;
        UserLayerDrawFunction DrawFunction;
    };

    // the full tilemap
    struct TileMap
    {
        std::map<uint16_t, TileSheet> TileSheets;		// the sheets used by this map
        std::vector<std::unique_ptr<LayerInfo>> Layers;	// the layers stored in draw order (back to front)

        Vector2 TileRenderOrder = { 1,1 };

        // used as a cache
        const TileSheet* LastTileSheet = nullptr;
    };

    /// <summary>
    /// Load a tile map from a file on disk
    /// </summary>
    /// <param name="filepath">The file path to load</param>
    /// <param name="map">The tile map to fill out</param>
    /// <returns>True if the file was loaded</returns>
    bool LoadTileMap(const std::string& filepath, TileMap& map);

    bool LoadTileMapFromMemory(const char* fileData, TileMap& map);

    /// <summary>
    /// Deallocates and clears a tilemap
    /// </summary>
    /// <param name="map">the map to clear</param>
    /// <param name="releaseTextures">When true, textures from all sheets will be unloaded from the GPU</param>
    void UnloadTileMap(TileMap& map, bool releaseTextures = true);

    // callback function that loads a texture, if not set default raylib functions will be used
    using LoadTextureFunction = std::function<Texture2D(const char* filePath)>;

    /// <summary>
    /// allow the game to install a custom reader function to get a texture from an existing system
    /// </summary>
    /// <param name="func">The callback function that will be used when a texture is needed</param>
    void SetLoadTextureFunction(LoadTextureFunction func);

    // callback function that loads a text file, if not set default raylib functions will be used
    using LoadTextFileFunction = std::function<std::string(const char* filePath)>;

    /// <summary>
    /// allow the game to install a custom reader function to get a text file from an existing system
    /// </summary>
    /// <param name="func">The callback function that will be used when a text file is needed</param>
    void SetLoadTextFileFunction(LoadTextFileFunction func);

    /// <summary>
    /// Insert a virtual layer into a tilemap
    /// </summary>
    /// <param name="map">The map to use<</param>
    /// <param name="beforeId">The layer ID to insert the virtual layer before</param>
    /// <returns>A new virtual layer with it's own unique ID that the game can fill out with data</returns>

    LayerInfo* InsertTileMapLayer(std::unique_ptr<LayerInfo> layer, TileMap& map, int beforeId);
    template<class T>
    T* InsertTileMapLayer(TileMap& map, int beforeId)
    {
        std::unique_ptr<T> layer = std::make_unique<T>();
        return static_cast<T*>(InsertTileMapLayer(std::move(layer), map, beforeId));
    }

    /// <summary>
    /// Remove a virtual layer from a map
    /// </summary>
    /// <param name="map">The map to use</param>
    /// <param name="layerId">the layer ID</param>
    /// <returns>Returns true if the layer was found and removed</returns>
    bool RemoveTileMapLayer(TileMap& map, int layerId);

    LayerInfo* FindLayer(TileMap& map, int layerId);
    LayerInfo* FindLayer(TileMap& map, const std::string& name);

    /// <summary>
    /// Draws all visible layers
    /// </summary>
    /// <param name="map">The Map to draw</param>
    /// <param name="camera">An optional camera, if provided only tiles in the visible range will be used</param>
    /// <param name="bounds">An optional size boundary, 
    /// if provided will be used with the camera to limit what is drawn, if not provided the screen size will be used
    /// </param>
    void DrawTileMap(TileMap& map, Camera2D* camera = nullptr, Vector2 bounds = { 0,0 });

    // draw stats
    size_t GetTileDrawStats();

    // TODO, general collision API
    struct CollisionRecord
    {
        TileLayerType Type = TileLayerType::Tile;
        Rectangle Bounds = { 0,0,0,0 };
        int32_t ItemId = 0;
    };

    size_t GetCollisions(TileMap& map, Rectangle rect, std::vector<CollisionRecord>& results);
}

