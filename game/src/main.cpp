/*
Doom Level Reader Test

*/

#include <stdint.h>
#include <string>
#include <vector>
#include <unordered_map>

#include "raylib.h"
#include "raymath.h"
#include "rcamera.h"
#include "rlgl.h"

#include "rlImGui.h"
#include "imgui.h"

#include "doom_map.h"
#include "doom_map_render.h"
#include "lump_inspectors.h"
#include "reader.h"

WADFile::LevelMap* Map;
RenderTexture SectorViewRT;

WADFile GameWad;

Camera2D MapViewCamera = { 0 };
constexpr float DefaultZoom = 5.0f;

Camera3D ViewCamera = { 0 };

size_t SelectedSector = 0;
size_t SelectedSubsector = 0;

bool View3D = false;

void SetCameraToSpawn()
{
	if (!Map || !Map->Things || Map->Things->ThingsByType[1].size() == 0)
		return;

	auto* spawn = *Map->Things->ThingsByType[1].begin();
	ViewCamera.position = { spawn->Position.x, spawn->Position.y, 1.5f };
	ViewCamera.target = ViewCamera.position;
	ViewCamera.target.z += 1;
}

void ShowLevelInfoWindow()
{
    if (ImGui::Begin("Map Info") && Map)
    {
		ImGui::TextUnformatted(Map->Name.c_str());

		ImGui::TextUnformatted("Sectors");
		if (ImGui::BeginListBox("##Sectors", ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing())))
		{
			for (size_t i = 0; i < Map->SectorCache.size(); i++)
			{
				const char* text = TextFormat("%d", i + 1);

				bool selected = SelectedSector == i;
				if (ImGui::Selectable(text, selected))
				{
					SelectedSector = i;
				}
			}

			ImGui::EndListBox();
		}

		ImGui::TextUnformatted("SubSectors");
		if (ImGui::BeginListBox("##SubSectors", ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing())))
		{
			for (size_t i = 0; i < Map->SectorCache[SelectedSector].SubSectors.size(); i++)
			{
				const char* text = TextFormat("%d", i + 1);

				bool selected = SelectedSubsector == i;
				if (ImGui::Selectable(text, selected))
				{
					SelectedSubsector = i;
				}
			}

			ImGui::EndListBox();
		}
    }
    ImGui::End();
}

void ShowGameInfoWindow()
{
    if (ImGui::Begin("Game WAD"))
    {
		ImGui::TextUnformatted("Maps");
		ImGui::SameLine();

		if (ImGui::RadioButton("2d", !View3D))
			View3D = false;
		ImGui::SameLine();
		if (ImGui::RadioButton("3d", View3D))
			View3D = true;

		if (ImGui::BeginListBox("###Maps", ImVec2(-FLT_MIN, 10 * ImGui::GetTextLineHeightWithSpacing())))
		{
			for (auto& level : GameWad.Levels)
			{
				bool selected = &level == Map;

				if (ImGui::Selectable(level.Name.c_str(), selected))
				{
					Map = &level;
					SelectedSector = 0;
					SelectedSubsector = 0;

					if (Map->Verts == nullptr)
						Map->Load();

					SetCameraToSpawn();
				}
			}

			ImGui::EndListBox();
		}

		ImGui::TextUnformatted("View Mode");
		
    }
    ImGui::End();
}

void DrawMapView()
{
	MapViewCamera.offset = Vector2{ GetScreenWidth() * 0.5f, (float)GetScreenHeight() * 0.5f };

	if (!Map || View3D)
		return;

	BeginTextureMode(SectorViewRT);
	ClearBackground(BLANK);

	BeginMode2D(MapViewCamera);
	DrawLine(-1, 0, 2, 0, RED);
	DrawLine(0, -1, 0, 2, GREEN);

	DoomRender::DrawMapSegs(*Map, SelectedSector, SelectedSubsector);

	EndMode2D();

	EndTextureMode();
	DrawTexture(SectorViewRT.texture, 0, 0, WHITE);
}

void Draw3DView()
{
	if (!Map || !View3D)
		return;

	BeginMode3D(ViewCamera);
	DrawCube(Vector3Zero(), 1, 1, 1, RED);

	DoomRender::DrawMap3d(*Map);

	EndMode3D();
}

void UpdateMapInput()
{
	float panSpeed = GetFrameTime() * View3D ? 0.5f : 1000;
	if (!ImGui::GetIO().WantCaptureKeyboard)
	{
		if (View3D)
		{
			if (IsKeyDown(KEY_W))
				CameraMoveForward(&ViewCamera, panSpeed, false);
			if (IsKeyDown(KEY_S))
				CameraMoveForward(&ViewCamera, -panSpeed, false);

 			if (IsKeyDown(KEY_E))
				CameraMoveUp(&ViewCamera, panSpeed);
 			if (IsKeyDown(KEY_Q))
				CameraMoveUp(&ViewCamera, -panSpeed);
		}
		else
		{
			if (IsKeyDown(KEY_W))
				MapViewCamera.target.y -= panSpeed;
			if (IsKeyDown(KEY_S))
				MapViewCamera.target.y += panSpeed;

			if (IsKeyDown(KEY_D))
				MapViewCamera.target.x -= panSpeed;
			if (IsKeyDown(KEY_A))
				MapViewCamera.target.x += panSpeed;
		}
	}

	if (!ImGui::GetIO().WantCaptureMouse)
	{
		if (View3D && IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
		{
			CameraYaw(&ViewCamera, GetMouseDelta().x * 0.001f, false);
			CameraPitch(&ViewCamera, GetMouseDelta().y * -0.001f, true, false, false);
		}
		else
		{
			float mouseScale = 1.0f / MapViewCamera.zoom;
			if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
				MapViewCamera.target = Vector2Add(MapViewCamera.target, Vector2{ GetMouseDelta().x * -mouseScale, GetMouseDelta().y * mouseScale });

			MapViewCamera.zoom += GetMouseWheelMove() * 0.0625f;
			if (MapViewCamera.zoom < DefaultZoom)
				MapViewCamera.zoom = DefaultZoom;
		}
	}
}

int main ()
{
	// set up the window
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(1280, 800, "Hello DOOM!");

	rlImGuiSetup(true);

	MapViewCamera.zoom = DefaultZoom;

	ViewCamera.fovy = 45;
	ViewCamera.up.z = 1;
	ViewCamera.position.y = -10;
	ViewCamera.position.z = 3;

	GameWad.Read("resources/glDOOMWAD.wad");

	if (GameWad.Levels.size() > 0)
		Map = &GameWad.Levels[0];

	if (Map)
		Map->Load();

	SetCameraToSpawn();

	SectorViewRT = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

	// game loop
	while (!WindowShouldClose())
	{
		if (IsWindowResized())
		{
			UnloadRenderTexture(SectorViewRT);
			SectorViewRT = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
		}

		UpdateMapInput();
		
		// drawing
		BeginDrawing();
		ClearBackground(BLACK);

		DrawMapView();
		Draw3DView();

		rlImGuiBegin();

		ShowLevelInfoWindow();
		ShowGameInfoWindow();
	//	ImGui::ShowDemoWindow();

		rlImGuiEnd();
		
		EndDrawing();
	}

	rlImGuiShutdown();
	// cleanup
	CloseWindow();
	return 0;
}