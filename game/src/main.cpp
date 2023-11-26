/*
Doom Level Reader Test

*/

#include <stdint.h>
#include <string>
#include <vector>
#include <unordered_map>

#include "raylib.h"
#include "raymath.h"

#include "rlImGui.h"
#include "imgui.h"

#include "doom_map.h"
#include "doom_map_render.h"
#include "lump_inspectors.h"
#include "reader.h"

WADFile::LevelMap* Map;

const WADData::DirectoryEntry* CurrentEntry = nullptr;

RenderTexture SectorViewRT;

WADFile GameWad;

Camera2D MapViewCamera = { 0 };
constexpr float DefaultZoom = 0.125f;

size_t SelectedSector = 0;
size_t SelectedSubsector = 0;


void AddChildNodes(size_t nodeId)
{

}

void ShowLevelInfoWindow()
{
    if (ImGui::Begin("Directory Entries") && Map)
    {
		ImGui::TextUnformatted("Lumps");

        if (ImGui::BeginListBox("##Entries", ImVec2(-FLT_MIN, 3 * ImGui::GetTextLineHeightWithSpacing())))
        {
            for (const auto& entry : Map->Entries)
            {
                bool selected = CurrentEntry == entry.second;

                if (ImGui::Selectable(entry.first.c_str(), selected))
                {
                    CurrentEntry = entry.second;
                }
            }

            ImGui::EndListBox();
        }
        if (CurrentEntry)
        {
            ImGui::Text("Lump Size = %d", int(CurrentEntry->LumpSize));
            ImGui::Text("Lump Offset = %d", int(CurrentEntry->LumpOffset));

            auto itr = Map->SourceWad.LumpDB.find(CurrentEntry->Name);
            if (itr != Map->SourceWad.LumpDB.end() && itr->second && itr->second->Visualize)
            {
                itr->second->Visualize(itr->second);
            }
        }

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

		ImGui::TextUnformatted("Nodes");
		if (ImGui::BeginListBox("##Nodes", ImVec2(-FLT_MIN, 20 * ImGui::GetTextLineHeightWithSpacing())))
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
		ImGui::TextUnformatted("Lumps");
        if (ImGui::BeginListBox("###GameEntries", ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing())))
        {
            for (const auto& entry : GameWad.Entries)
            {
                bool selected = false;

                if (ImGui::Selectable(entry.second.Name.c_str(), selected))
                {

                }
            }

            ImGui::EndListBox();
        }
		ImGui::TextUnformatted("Maps");
		if (ImGui::BeginListBox("###Maps", ImVec2(-FLT_MIN, 10 * ImGui::GetTextLineHeightWithSpacing())))
		{
			for (auto& level : GameWad.Levels)
			{
				bool selected = &level == Map;

				if (ImGui::Selectable(level.Name.c_str(), selected))
				{
					Map = &level;
					SelectedSector = 0;

					if ( Map->SourceWad.LumpDB.size() == 0)
						Map->Load();
				}
			}

			ImGui::EndListBox();
		}
    }
    ImGui::End();
}

void DrawLevelLines()
{
	MapViewCamera.offset = Vector2{ GetScreenWidth() * 0.5f, (float)GetScreenHeight() * 0.5f };

	BeginTextureMode(SectorViewRT);
	ClearBackground(BLANK);

	BeginMode2D(MapViewCamera);
	DrawLine(-25, 0, 100, 0, RED);
	DrawLine(0, -25, 0, 100, GREEN);

// 	if (Map)
// 		DoomRender::DrawMapSectorPolygons(*Map, SelectedSector);
	if (Map)
		DoomRender::DrawMapSegs(*Map, SelectedSector, SelectedSubsector);

// 	if (Map)
// 		DoomRender::DrawMapNodes(*Map, SelectedSector, SelectedSubsector);

	EndMode2D();

	EndTextureMode();
	DrawTexture(SectorViewRT.texture, 0, 0, WHITE);
}

void UpdateMapInput()
{
	float panSpeed = GetFrameTime() * 1000;
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

		MapViewCamera.zoom += GetMouseWheelMove() * 0.0625f;
		if (MapViewCamera.zoom < DefaultZoom)
			MapViewCamera.zoom = DefaultZoom;
	}
}

int main ()
{
	// set up the window
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(1280, 800, "Hello DOOM!");

	rlImGuiSetup(true);

	MapViewCamera.zoom = DefaultZoom;

	GameWad.Read("resources/glDOOMWAD.wad");

	if (GameWad.Levels.size() > 0)
		Map = &(*GameWad.Levels.begin());

	if (Map)
		Map->Load();

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

		DrawLevelLines();

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