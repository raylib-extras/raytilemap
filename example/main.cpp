/**********************************************************************************************
*
*   RayTileMap Example
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

#include "raylib.h"
#include "raymath.h"

#include "ray_tilemap.h"

using namespace RayTiled;

TileMap Map;

Camera2D ViewCamera = { 0 };

void GameInit()
{
	ViewCamera.zoom = 1;

	LoadTileMap("resources/sample_map.tmx", Map);
}

bool GameUpdate()
{
	if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
	{
		ViewCamera.target = Vector2Subtract(ViewCamera.target, GetMouseDelta());
	}
	return true;
}

void GameDraw()
{
	BeginDrawing();
	ClearBackground(DARKGRAY);

	BeginMode2D(ViewCamera);
	DrawTileMap(Map, &ViewCamera);
	EndMode2D();

	EndDrawing();
}

int main()
{
	SetConfigFlags(FLAG_VSYNC_HINT);
	InitWindow(1280, 800, "Example");
	SetTargetFPS(144);

	GameInit();

	while (!WindowShouldClose())
	{
		if (!GameUpdate())
			break;
		
		GameDraw();
	}

	CloseWindow();
	return 0;
}