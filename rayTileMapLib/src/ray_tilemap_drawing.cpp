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

    const TileSheet* FindSheetForId(uint16_t id, const TileMap& map)
    {
        for (const auto& [startId, sheet] : map.TileSheets)
        {
            if (sheet.HasId(id))
                return &sheet;
        }

        return nullptr;
    }

    void DrawTileMap(TileMap& map, Camera2D* camera, Vector2 bounds)
    {
        const TileSheet* lastTileSheet = &map.TileSheets.begin()->second;
        for (const auto& layer : map.Layers)
        {
            if (layer->Type == TileLayerType::Tile)
            {
                const TileLayer* tileLayer = static_cast<TileLayer*>(layer.get());
                for (int y = 0; y < tileLayer->Bounds.y; y++)
                {
                    for (int x = 0; x < tileLayer->Bounds.x; x++)
                    {
                        Rectangle destRect;
                        const auto* tile = tileLayer->GetTile(x, y, destRect);
                        if (tile == nullptr || tile->TileIndex == 0)
                            continue;

                        if (lastTileSheet == nullptr || !lastTileSheet->HasId(tile->TileIndex))
                            lastTileSheet = FindSheetForId(tile->TileIndex, map);

                        if (!lastTileSheet)
                            continue;

                        lastTileSheet->DrawTile(tile->TileIndex, destRect, tile->TileFlags, WHITE);
                    }
                }
            }
        }
    }
}
