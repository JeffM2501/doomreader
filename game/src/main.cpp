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

void ShowLevelInfoWindow()
{
    if (ImGui::Begin("Directory Entries") && Map)
    {
        if (ImGui::BeginListBox("##Entries", ImVec2(-FLT_MIN, 10 * ImGui::GetTextLineHeightWithSpacing())))
        {
            for (const auto& entry : Map->Entries)
            {
                bool selected = CurrentEntry == &entry;

                if (ImGui::Selectable(entry.Name.c_str(), selected))
                {
                    CurrentEntry = &entry;
                }
            }

            ImGui::EndListBox();
        }
        if (CurrentEntry)
        {
            ImGui::Text("Lump Size = %d", int(CurrentEntry->LumpSize));
            ImGui::Text("Lump Offset = %d", int(CurrentEntry->LumpOffset));

            auto itr = Map->LumpDB.find(CurrentEntry->Name);
            if (itr != Map->LumpDB.end() && itr->second && itr->second->Visualize)
            {
                itr->second->Visualize(itr->second);
            }
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

                if (ImGui::Selectable(entry.Name.c_str(), selected))
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

					if ( Map->LumpDB.size() == 0)
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
	Camera2D cam = { 0 };
	cam.zoom = 0.125f;
	cam.offset = Vector2{ GetScreenWidth() * 0.5f, (float)GetScreenHeight() * 0.5f };

	BeginTextureMode(SectorViewRT);
	ClearBackground(BLANK);

	BeginMode2D(cam);
	DrawLine(-100, 0, 100, 0, RED);
	DrawLine(0, -100, 0, 100, GREEN);

	if (Map)
		DoomRender::DrawMapLines(*Map);
	EndMode2D();

	EndTextureMode();
	DrawTexture(SectorViewRT.texture, 0, 0, WHITE);
}

int main ()
{
	// set up the window
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(1280, 800, "Hello DOOM!");

	rlImGuiSetup(true);

	GameWad.Read("resources/DOOM.wad");

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