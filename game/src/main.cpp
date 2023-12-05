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
#include "camera_controller.h"

WADFile::LevelMap* Map;
RenderTexture SectorViewRT;

WADFile GameWad;

Camera2D MapViewCamera = { 0 };
constexpr float DefaultZoom = 5.0f;

Camera3D ViewCamera = { 0 };
CameraController Controller;

size_t SelectedSector = 0;
size_t SelectedSubsector = 0;

bool View3D = false;

void SetCameraToSpawn()
{
	if (!Map || !Map->Things || Map->Things->ThingsByType[1].size() == 0)
		return;
	auto* spawn = *Map->Things->ThingsByType[1].begin();
	Controller.SetPosition(Vector3{ spawn->Position.x, spawn->Position.y , 0});
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
			if (SelectedSector != size_t(-1))
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
			}

			ImGui::EndListBox();
		}
    }
    ImGui::End();
}

void LoadMap()
{
    SelectedSector = 0;
    SelectedSubsector = 0;

    if (Map->Verts == nullptr)
        Map->Load();

    SetupLumpInspector(WADData::THINGS, Map->Things);
    SetupLumpInspector(WADData::SSECTORS, Map->Subsectors);
    SetupLumpInspector(WADData::LINEDEFS, Map->Lines);
    SetupLumpInspector(WADData::SIDEDEFS, Map->Sides);
    SetupLumpInspector(WADData::SECTORS, Map->Sectors);

    SetCameraToSpawn();
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
					LoadMap();
				}
			}

			ImGui::EndListBox();
		}
    }
    ImGui::End();
}

void ShowMapLumpInspector()
{
    if (ImGui::Begin("Map Lumps") && Map)
    {
        if (Map->Sectors)
        {
            ImGui::TextUnformatted("Sectors");
            Map->Sectors->Visualize(Map->Sectors, Map);
        }


		if (Map->Lines)
		{
			ImGui::TextUnformatted("LineDefs");
			Map->Lines->Visualize(Map->Lines, Map);
		}

		if (Map->Sides)
		{
			ImGui::TextUnformatted("SideDefs");
			Map->Sides->Visualize(Map->Sides, Map);
		}
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

	Controller.SetCamera(ViewCamera);

	BeginMode3D(ViewCamera);
	DrawCube(Vector3Zero(), 1, 1, 1, RED);
	DoomRender::DrawMap3d(*Map);
	EndMode3D();
}

void UpdateMapInput()
{
	if (View3D)
	{
		if (!ImGui::GetIO().WantCaptureKeyboard && !ImGui::GetIO().WantCaptureMouse)
			Controller.Update(*Map);
	}
	else
	{
		float panSpeed = GetFrameTime() * View3D ? 0.5f : 1000;
		if (!ImGui::GetIO().WantCaptureKeyboard)
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

		if (!ImGui::GetIO().WantCaptureMouse)
		{
			float mouseScale = 1.0f / MapViewCamera.zoom;
			if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
				MapViewCamera.target = Vector2Add(MapViewCamera.target, Vector2{ GetMouseDelta().x * -mouseScale, GetMouseDelta().y * mouseScale });

			float wheelScrollFactor = 0.0625f;
			if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
				wheelScrollFactor *= 10;

			MapViewCamera.zoom += GetMouseWheelMove() * wheelScrollFactor;
			if (MapViewCamera.zoom < DefaultZoom)
				MapViewCamera.zoom = DefaultZoom;

			Vector2 rtMousePos = { GetMousePosition().x, GetScreenHeight() - GetMousePosition().y };
			Vector2 worldCamera = GetScreenToWorld2D(rtMousePos, MapViewCamera);

			if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
			{
				SelectedSector = Map->GetSectorFromPoint(worldCamera.x, worldCamera.y, &SelectedSubsector);
			}

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
	Controller.SetPosition(Vector3{ 0,-2,0 });
	
	GameWad.Read("resources/glDOOMWAD.wad");

	if (GameWad.Levels.size() > 0)
		Map = &GameWad.Levels[0];

	LoadMap();

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
		ShowMapLumpInspector();
	//	ImGui::ShowDemoWindow();

		rlImGuiEnd();
		
		EndDrawing();
	}

	rlImGuiShutdown();
	// cleanup
	CloseWindow();
	return 0;
}