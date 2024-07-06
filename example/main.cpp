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

UserLayer* TestUserLayer = nullptr;

TileLayer* ObjectTileLayer = nullptr;

struct PlayerDrawable : TileLayer::Drawable
{
	Vector2 Position = { 300,300 };

	float Radius = 8;

	float GetY() override { return Position.y - Radius; }
};

PlayerDrawable Player;

void DrawUserLayer(UserLayer& layer, Camera2D* camera, Vector2 bounds)
{
	DrawText("I AM IN A USER LAYER!!!", 200, 150, 40, DARKPURPLE);
}

void DrawObjectLayerItem(TileLayer& layer, TileLayer::Drawable& drawable, float startX, float endX)
{
	DrawCircleV(Player.Position, Player.Radius, MAGENTA);
}

void GameInit()
{
	ViewCamera.zoom = 1;

	LoadTileMap("resources/sample_map.tmx", Map);

	TestUserLayer = InsertTileMapLayer<UserLayer>(Map, Map.Layers.back()->LayerId);
	TestUserLayer->DrawFunction = DrawUserLayer;

	auto playerLayer = FindLayer(Map, "Objects");
	if (playerLayer && playerLayer->Type == TileLayerType::Tile)
	{
		ObjectTileLayer = static_cast<TileLayer*>(playerLayer);

		ObjectTileLayer->CustomDrawalbeFunction = DrawObjectLayerItem;
		ObjectTileLayer->AddDrawable(&Player);
	}
}

bool GameUpdate()
{
	if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
	{
		ViewCamera.target = Vector2Subtract(ViewCamera.target, GetMouseDelta());
	}

	float speed = 200 * GetFrameTime();

	if (IsKeyDown(KEY_W))
		Player.Position.y -= speed;
    if (IsKeyDown(KEY_S))
        Player.Position.y += speed;

    if (IsKeyDown(KEY_A))
        Player.Position.x -= speed;
    if (IsKeyDown(KEY_D))
        Player.Position.x += speed;

	return true;
}

void GameDraw()
{
	BeginDrawing();
	ClearBackground(DARKGRAY);

	BeginMode2D(ViewCamera);
	DrawTileMap(Map, &ViewCamera);

	DrawLine(0, 0, 100, 0, RED);
    DrawLine(0, 0, 0, 100, BLUE);

	EndMode2D();

	DrawFPS(5, 5);
	DrawText(TextFormat("Tiles Drawn: %d", (int)GetTileDrawStats()), 5, 25, 20, WHITE);

	EndDrawing();
}

int main()
{
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
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