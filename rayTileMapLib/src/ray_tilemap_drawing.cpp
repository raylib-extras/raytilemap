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

#include "ray_tilemap.h"
#include "external/PUGIXML/pugixml.hpp"

namespace RayTiled
{
    Texture2D GetTexture(const std::string& fileName);

    void TileSheet::DrawTile(uint16_t id, Rectangle destinationRectangle, uint8_t flags, Color tint) const
    {
        Rectangle sourceRect = Tiles[id - StartingTileId];
        float rotation = 0;

        if (flags & TileFlagsFlipDiagonal)
            rotation -= 90;
        if (flags & TileFlagsFlipHorizontal)
            sourceRect.width *= -1;
        if (flags & TileFlagsFlipVertical)
            sourceRect.height *= -1;

        DrawTexturePro(Texture, sourceRect, destinationRectangle, Vector2Zero(), rotation, tint);
    }

    const TileInfo* TileLayer::GetTile(int x, int y, Rectangle& screenRect) const
    {
        if (x > Bounds.x || x < 0 || y > Bounds.y || y < 0)
            return nullptr;

        screenRect.x = x * TileSize.x;
        screenRect.y = y * TileSize.y;
        screenRect.width = TileSize.x;
        screenRect.height = TileSize.y;

        return &TileData[y * int(Bounds.x) + x];
    }

    bool TileLayer::CellHasTile(int x, int y, uint16_t* result)
    {
        if (x > Bounds.x || x < 0 || y > Bounds.y || y < 0)
            return false;

        if (result)
            *result = TileData[y * int(Bounds.x) + x].TileIndex;

        return TileData[y * int(Bounds.x) + x].TileIndex > 0;
    }

    const TileSheet* FindSheetForId(uint16_t id, const TileMap& map)
    {
        for (const auto& [startId, sheet] : map.TileSheets)
        {
            if (sheet.HasId(id))
                return &sheet;
        }

        return nullptr;
    }

    const TileSheet* LastTileSheet = nullptr;

    static size_t TilesDrawn = 0;

    size_t GetTileDrawStats()
    {
        return TilesDrawn;
    }

    void DrawTileLayer(TileMap& map, TileLayer* tileLayer, Camera2D* camera, Vector2 bounds)
    {
        int startX = 0;
        int startY = 0;

        int endX = int(tileLayer->Bounds.x);
        int endY = int(tileLayer->Bounds.y);

        int xDirection = 1;
        int yDirection = 1;

        if (camera)
        {
            if (bounds.x <= 0 || bounds.y <= 0)
            {
                bounds.x = (float)GetScreenWidth();
                bounds.y = (float)GetScreenHeight();
            }
            Vector2 viewportStart = GetScreenToWorld2D(Vector2Zero(), *camera);
            Vector2 viewportEnd = GetScreenToWorld2D(bounds, *camera);

            startX = std::max(startX, int(viewportStart.x / tileLayer->TileSize.x) - 1);
            startY = std::max(startY, int(viewportStart.y / tileLayer->TileSize.y) - 1);

            endX = std::min(endX, int(viewportEnd.x / tileLayer->TileSize.x) + 2);
            endY = std::min(endY, int(viewportEnd.y / tileLayer->TileSize.y) + 2);
        }

        // Handle the direction stuff from the map file

        for (int y = startY; y != endY; y+= yDirection)
        {
            for (int x = startX; x != endX; x += xDirection)
            {
                Rectangle destRect;
                const auto* tile = tileLayer->GetTile(x, y, destRect);
                if (tile == nullptr || tile->TileIndex == 0)
                    continue;

                if (LastTileSheet == nullptr || !LastTileSheet->HasId(tile->TileIndex))
                    LastTileSheet = FindSheetForId(tile->TileIndex, map);

                if (!LastTileSheet)
                    continue;

                LastTileSheet->DrawTile(tile->TileIndex, destRect, tile->TileFlags, WHITE);
                TilesDrawn++;
            }
            
            if (tileLayer->CustomDrawalbeFunction)
            {
                for (auto& drawable : tileLayer->Drawables)
                {
                    float yPos = drawable->GetY();
                    if (yPos > y * tileLayer->TileSize.y && yPos <= (y + 1) * tileLayer->TileSize.y)
                        tileLayer->CustomDrawalbeFunction(*tileLayer, *drawable, startX * tileLayer->TileSize.x, endX * tileLayer->TileSize.x);
                }
            }
        }
    }

    void DrawVirtualLayer(TileMap& map, UserLayer* virtualLayer, Camera2D* camera, Vector2 bounds)
    {
        if (virtualLayer && virtualLayer->DrawFunction)
            virtualLayer->DrawFunction(*virtualLayer, camera, bounds);
    }

    void DrawTileMap(TileMap& map, Camera2D* camera, Vector2 bounds)
    {
        TilesDrawn = 0;
        for (auto& layer : map.Layers)
        {
            switch (layer->Type)
            {
            default:
                break;
            case TileLayerType::Tile:
                DrawTileLayer(map, static_cast<TileLayer*>(layer.get()), camera, bounds);
                break;
            case TileLayerType::Object:
            {
                auto objectLayer = static_cast<ObjectLayer*>(layer.get());
                if (objectLayer && objectLayer->DrawFunc)
                    objectLayer->DrawFunc(*objectLayer, camera, bounds);
            }
            break;
            case TileLayerType::User:
                DrawVirtualLayer(map, static_cast<UserLayer*>(layer.get()), camera, bounds);
                break;
            }
        }
    }
}
