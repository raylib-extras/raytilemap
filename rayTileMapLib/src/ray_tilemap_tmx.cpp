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
    pugi::xml_parse_result ParseXML(const std::string& fileName, pugi::xml_document& doc);
    bool ReadTiledXML(pugi::xml_document& doc, TileMap& map);
    void SetFolderPath(const std::string& path);
    void ClearFolderPath();
    Texture2D GetTexture(const std::string& fileName);

    bool LoadTileMap(const std::string& filepath, TileMap& map)
    {
        map.TileSheets.clear();
        map.Layers.clear();

        pugi::xml_document doc;
        auto result = ParseXML(filepath, doc);

        SetFolderPath(GetDirectoryPath(filepath.c_str()));

        bool ret = result.status == pugi::xml_parse_status::status_ok && ReadTiledXML(doc, map);
        ClearFolderPath();
        return ret;
    }

    bool LoadTileMapFromMemory(const char* fileData, TileMap& map)
    {
        map.TileSheets.clear();
        map.Layers.clear();

        if (fileData == nullptr)
            return false;

        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_string(fileData);
        return result.status == pugi::xml_parse_status::status_ok && ReadTiledXML(doc, map);
    }

    const unsigned FLIPPED_HORIZONTALLY_FLAG = 0x80000000;
    const unsigned FLIPPED_VERTICALLY_FLAG = 0x40000000;
    const unsigned FLIPPED_DIAGONALLY_FLAG = 0x20000000;

    static bool ReadImageData(int& width, int& height, std::string& source, pugi::xml_node image)
    {
        if (image.root() == nullptr)
            return false;

        width = image.attribute("width").as_int();
        height = image.attribute("height").as_int();

        source = image.attribute("source").as_string();

        if (source.size() > 0)
        {
            if (source[0] == '.')
            {
                size_t firstSlash = source.find_first_of('/');
                if (firstSlash != std::string::npos)
                    source = source.substr(firstSlash + 1);
            }
        }
        return true;
    }

    bool ReadTileSetNode(pugi::xml_node root, int idOffset, TileMap& map)
    {
        float tileWidth = root.attribute("tilewidth").as_float();
        float tileHeight = root.attribute("tileheight").as_float();

        int tileCount = root.attribute("tilecount").as_int();

        int guid = root.attribute("firstgid").as_int();

        int columCount = root.attribute("columns").as_int();
        int spacing = root.attribute("spacing").as_int();
        int margin = root.attribute("margin").as_int();
        std::string source;

        int width, height;

        for (pugi::xml_node child : root.children())
        {
            std::string n = child.name();
            if (n == "tile")
            {
                int id = child.attribute("id").as_int();
                if (!ReadImageData(width, height, source, child.child("image")))
                    continue;
            }
            else if (n == "image")
            {
                ReadImageData(width, height, source, child);
            }
        }

        map.TileSheets.try_emplace(guid);

        auto& tilesheet = map.TileSheets[guid];
        tilesheet.Texture = GetTexture(source);
        tilesheet.StartingTileId = guid;

        for (int y = margin; y < height - margin; y += int(tileHeight) + spacing)
        {
            for (int x = margin; x < width - margin; x += int(tileWidth) + spacing)
            {
                tilesheet.Tiles.emplace_back(Rectangle{ float(x), float(y), tileWidth, tileHeight });
            }
        }

        return true;
    }

    bool ReadTileSetFile(const std::string& tilesetFileName, int idOffset, TileMap& map)
    {
        pugi::xml_document doc;

        pugi::xml_parse_result result = ParseXML(tilesetFileName, doc);

        if (result.status != pugi::xml_parse_status::status_ok)
            return false;

        pugi::xml_node root = doc.child("tileset");

        return ReadTileSetNode(root, idOffset, map);
    }

    std::vector<std::string> split(const char* str, char c = ' ')
    {
        std::vector<std::string> result;

        do
        {
            const char* begin = str;

            while (*str != c && *str)
                str++;

            result.push_back(std::string(begin, str));
        } while (0 != *str++);

        return result;
    }

    bool ReadObjectsLayer(pugi::xml_node& root, TileMap& map)
    {
//         std::shared_ptr<ObjectLayer> layerPtr = std::make_shared<ObjectLayer>();
// 
//         ObjectLayer& layer = *layerPtr;
//         int id = root.attribute("id").as_int();
// 
//         layer.Id = id;
//         layer.Name = root.attribute("name").as_string();
// 
//         for (pugi::xml_node child : root.children())
//         {
//             std::string n = child.name();
//             if (n == "object")
//             {
//                 int id = child.attribute("id").as_int();
// 
//                 std::shared_ptr<TileObject> object = nullptr;
// 
//                 if (!child.child("polygon").empty() || !child.child("polyline").empty())
//                 {
//                     auto poly = std::make_shared<TilePolygonObject>();
// 
//                     auto points = split(child.child("polygon").attribute("points").as_string(), ' ');
//                     for (auto point : points)
//                     {
//                         auto coords = split(point.c_str(), ',');
//                         if (coords.size() == 2)
//                         {
//                             Vector2 p = { (float)atof(coords[0].c_str()), (float)atof(coords[1].c_str()) };
//                             poly->Points.emplace_back(p);
//                         }
//                     }
//                     object = poly;
// 
//                 }
//                 else if (!child.child("text").empty())
//                 {
//                     auto text = std::make_shared<TileTextObject>();
//                     auto textEntity = child.child("text");
// 
//                     text->Text = textEntity.child_value();
//                     if (!textEntity.attribute("pixelsize").empty())
//                         text->FontSize = textEntity.attribute("pixelsize").as_int();
// 
//                     // TODO, add the rest of the text attributes
// 
//                     object = text;
//                 }
//                 else
//                 {
//                     object = std::make_shared<TileObject>();
//                 }
// 
//                 if (!child.child("polygon").empty())
//                     object->SubType = TileObject::SubTypes::Polygon;
//                 else if (!child.child("polyline").empty())
//                     object->SubType = TileObject::SubTypes::Polyline;
//                 else if (!child.child("ellipse").empty())
//                     object->SubType = TileObject::SubTypes::Ellipse;
//                 else if (!child.child("text").empty())
//                     object->SubType = TileObject::SubTypes::Text;
//                 else if (!child.child("point").empty())
//                     object->SubType = TileObject::SubTypes::Point;
//                 else
//                     object->SubType = TileObject::SubTypes::None;
// 
//                 object->Name = child.attribute("name").as_string();
//                 object->Type = child.attribute("type").as_string();
//                 object->Template = child.attribute("template").as_string();
// 
//                 object->Bounds.x = child.attribute("x").as_float();
//                 object->Bounds.y = child.attribute("y").as_float();
//                 object->Bounds.width = child.attribute("width").as_float();
//                 object->Bounds.height = child.attribute("height").as_float();
//                 object->Rotation = child.attribute("rotation").as_float();
//                 object->Visible = child.attribute("visible").empty() || child.attribute("visible").as_int() != 0;
// 
//                 object->GridTile = child.attribute("gid").as_int();
// 
//                 auto properties = child.child("properties");
//                 if (!properties.empty())
//                 {
//                     for (auto prop : properties.children())
//                     {
//                         Property propertyRectord;
//                         propertyRectord.Name = prop.attribute("name").as_string();
//                         propertyRectord.Type = prop.attribute("type").as_string();
//                         propertyRectord.Value = prop.attribute("value").as_string();
// 
//                         object->Properties.emplace_back(std::move(propertyRectord));
//                     }
//                 }
// 
//                 layer.Objects.emplace_back(object);
//             }
//         }
// 
//         int index = int(map.Layers.size());
//         map.Layers[index] = layerPtr;
//         map.ObjectLayers[index] = layerPtr.get();
        return true;
    }

    void AddLayerTile(TileLayer* layer, uint32_t val)
    {
        layer->TileData.emplace_back();
        TileInfo& tile = layer->TileData.back();
        if (val & FLIPPED_HORIZONTALLY_FLAG)
            tile.TileFlags |= TileFlagsFlipHorizontal;

        if (val & FLIPPED_VERTICALLY_FLAG)
            tile.TileFlags |= TileFlagsFlipVertical;

        if (val & FLIPPED_DIAGONALLY_FLAG)
            tile.TileFlags |= TileFlagsFlipDiagonal;

        val &= ~(FLIPPED_HORIZONTALLY_FLAG | FLIPPED_VERTICALLY_FLAG | FLIPPED_DIAGONALLY_FLAG);

        // subtract 1 from the index, since our sprites start at 0 not 1
        tile.TileIndex = static_cast<uint16_t>(val);
    }

    bool ReadTiledXML(pugi::xml_document& doc, TileMap& map)
    {
        auto root = doc.child("map");

        auto version = root.attribute("version");
        auto tiledVersion = root.attribute("tiledversion");

        std::string orient = root.attribute("orientation").as_string();
        std::string renderorder = root.attribute("renderorder").as_string();

        if (orient != "orthogonal")
            return false;

        int width = root.attribute("width").as_int();
        int height = root.attribute("height").as_int();

        int tilewidth = root.attribute("tilewidth").as_int();
        int tileheight = root.attribute("tileheight").as_int();

        for (auto child : root.children())
        {
            std::string childName = child.name();
            if (childName == "tileset")
            {
                int idOffset = 0;
                if (!child.attribute("firstgid").empty())
                    idOffset = child.attribute("firstgid").as_int();

                std::string tilesetFile = child.attribute("source").as_string();
                if (tilesetFile.size() == 0)
                {
                    if (!ReadTileSetNode(child, idOffset, map))
                        return false;
                }
                else if (!ReadTileSetFile(tilesetFile, idOffset, map))
                {
                    return false;
                }
            }
            else if (childName == "properties")
            {
//                 if (!child.empty())
//                 {
//                     for (auto prop : child.children())
//                     {
//                         Property propertyRectord;
//                         propertyRectord.Name = prop.attribute("name").as_string();
//                         propertyRectord.Type = prop.attribute("type").as_string();
//                         propertyRectord.Value = prop.attribute("value").as_string();
// 
//                         map.Properties.emplace_back(std::move(propertyRectord));
//                     }
//                 }
            }
            else if (childName == "objectgroup")
            {
                ReadObjectsLayer(child, map);
            }
            else if (childName == "layer")
            {
                int index = int(map.Layers.size());
                int layerID = child.attribute("id").as_int();

                std::string name = child.attribute("name").as_string();

                std::unique_ptr<TileLayer> layerPtr = std::make_unique<TileLayer>();
                TileLayer* layer = layerPtr.get();
                map.Layers.emplace_back(std::move(layerPtr));

                layer->Name = name;
                layer->LayerId = layerID;
                layer->Bounds.x = float(width);
                layer->Bounds.y = float(height);
                layer->TileSize.x = float(tilewidth);
                layer->TileSize.y = float(tileheight);

                auto data = child.child("data");
                std::string encoding = data.attribute("encoding").as_string();
                std::string contents = data.first_child().value();
                if (encoding == "csv")
                {
                    std::vector<std::vector<int>> rawData;

                    size_t linePos = 0;
                    do
                    {
                        size_t nextLine = contents.find_first_of('\n', linePos);
                        if (nextLine == std::string::npos)
                            nextLine = contents.size();

                        std::string colText = contents.substr(linePos, nextLine - linePos);
                        linePos = nextLine + 1;

                        if (colText.size() == 0)
                            continue;
                        size_t charPos = 0;
                        do
                        {
                            size_t nextDelim = colText.find_first_of(',', charPos);
                            if (nextDelim == std::string::npos || nextDelim == colText.size() - 1)
                                nextDelim = colText.size();

                            std::string valStr = colText.substr(charPos, nextDelim - charPos);
                            uint32_t val = static_cast<uint32_t>(std::atoll(valStr.c_str()));
                            charPos = nextDelim + 1;

                            AddLayerTile(layer, val);
                        } while (charPos <= colText.size());
                    } while (linePos < contents.size() && linePos != std::string::npos);
                }
                else if (encoding == "base64")
                {
                    std::string compression = data.attribute("compression").as_string();
                    int size = 0;

                    uint32_t header = *(uint32_t*)(contents.data());

                    uint8_t* data = DecodeDataBase64((const unsigned char*)contents.c_str() + 4, &size);
                   
                    uint32_t* decompData = nullptr;
                    int decompSize = 0;

                    if (compression == "zlib")
                    {
                        // TODO, decompress this
                        MemFree(data);
                        continue;
                    }
                    else if (compression == "gzip")
                    {
                        // TODO, decompress this
                        MemFree(data);
                        continue;
                    }
                    else if (compression == "zstd")
                    {
                        // TODO, decompress this
                        continue;
                        MemFree(data);
                    }
                    else
                    {
                        decompData = (uint32_t*)data;
                        decompSize = size / 4;
                    }
             
                    if (decompData && decompSize)
                    {
                        for (int index = 0; index < decompSize; index++)
                        {
                            AddLayerTile(layer, decompData[index]);
                        }

                        MemFree(decompData);
                    }
                }
            }
        }

        return map.TileSheets.size() > 0;
    }

}
