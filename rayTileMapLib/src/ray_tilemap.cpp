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
    static LoadTextureFunction LoadTextureFunc = nullptr;
    static LoadTextFileFunction LoadTextFileFunc = nullptr;

    std::string FolderPath;
    void SetLoadTextureFunction(LoadTextureFunction func)
    {
        LoadTextureFunc = func;
    }

    void SetLoadTextFileFunction(LoadTextFileFunction func)
    {
        LoadTextFileFunc = func;
    }

    void SetFolderPath(const std::string& path)
    {
        FolderPath = path;
    }

    void ClearFolderPath()
    {
        FolderPath.clear();
    }

    Texture2D GetTexture(const std::string& fileName)
    {
        std::string fullpath = fileName;
        if (!FolderPath.empty())
            fullpath = (FolderPath + "/" + fileName);

        if (LoadTextureFunc)
            return LoadTextureFunc(fullpath.c_str());

        return LoadTexture(fullpath.c_str());
    }

    pugi::xml_parse_result ParseXML(const std::string& fileName, pugi::xml_document& doc)
    {
        pugi::xml_parse_result result;

        std::string fullpath = fileName;
        if (!FolderPath.empty())
            fullpath = (FolderPath + "/" + fileName);
        if (LoadTextFileFunc)
        {
            result = doc.load_string(LoadTextFileFunc(fullpath.c_str()).c_str());
        }
        else
        {
            char* data = LoadFileText(fullpath.c_str());
            result = doc.load_string(data);
            UnloadFileText(data);
        }
        return result;
    }
}
