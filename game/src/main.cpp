/*
Raylib example file.
This is an example main file for a simple raylib project.
Use this as a starting point or replace it with your code.

For a C++ project simply rename the file to .cpp and run premake 

*/

#include <stdint.h>
#include <string>
#include <vector>
#include <unordered_map>

#include "raylib.h"
#include "raymath.h"
#include "reader.h"
#include "rlImGui.h"
#include "imgui.h"
#include "lump_inspectors.h"

std::vector<WADData::DirectoryEntry> Entries;
std::unordered_map<std::string, WADData::Lump*> LumpDB;

uint8_t* DataBuffer = nullptr;

const WADData::DirectoryEntry* CurrentEntry = nullptr;

RenderTexture SectorViewRT;


void LoadLumpData(const WADData::DirectoryEntry* entry)
{
	if (!entry|| LumpDB.find(entry->Name) != LumpDB.end())
		return;

	WADData::Lump* lump = WADData::GetLump(entry->Name);
	if (!lump)
		return;

	lump->Parse(DataBuffer, entry->LumpOffset, entry->LumpSize);
	LumpDB.insert_or_assign(entry->Name, lump);

	SetupLumpInspector(entry->Name, lump);
}

void ReadLevelWad()
{
    int size = 0;
    DataBuffer = LoadFileData("resources/E1M1.wad", &size);

    Entries = WADReader::ReadDirectoryEntries(DataBuffer);

	for (auto& ent : Entries)
		LoadLumpData(&ent);
}

void ClearLumpData()
{
	for (const auto& [key, lump] : LumpDB)
	{
		if (lump)
			delete(lump);
	}

	LumpDB.clear();
}

void DrawLevelLines()
{
	Camera2D cam = { 0 };
	cam.zoom = 0.125f;
	cam.offset = Vector2{ GetScreenWidth() * 0.5f, (float)GetScreenHeight() - 50 };

	BeginTextureMode(SectorViewRT);
	ClearBackground(BLANK);

	BeginMode2D(cam);
	DrawLine(-100, 0, 100, 0, RED);
	DrawLine(0, -100, 0, 100, GREEN);

	WADData::VertexesLump* verts = (WADData::VertexesLump*)LumpDB[WADData::VERTEXES];
	WADData::LineDefLump* lines = (WADData::LineDefLump*)LumpDB[WADData::LINEDEFS];

	for (const auto& line : lines->Contents)
	{
		auto& sp = verts->Contents[line.Start];
		auto& ep = verts->Contents[line.End];

		DrawLine(sp.X, sp.Y, ep.X, ep.Y, WHITE);
	}

	WADData::ThingsLump* things = (WADData::ThingsLump*)LumpDB[WADData::THINGS];
	for (const auto& thing : things->Contents)
	{
		DrawCircle(thing.X, thing.Y, 10, YELLOW);
	}
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

	ReadLevelWad();

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

		if (ImGui::Begin("Directory Entries"))
		{
			if (ImGui::BeginListBox("##Entries", ImVec2(-FLT_MIN, 10 * ImGui::GetTextLineHeightWithSpacing())))
			{
				for (const auto& entry : Entries)
				{
					bool selected = CurrentEntry == &entry;

					if (ImGui::Selectable(entry.Name.c_str(), selected))
					{
						CurrentEntry = &entry;
					}
				}

				ImGui::EndListBox();
			}
		}

		if (CurrentEntry)
		{
			ImGui::Text("Lump Size = %d", int(CurrentEntry->LumpSize));
			ImGui::Text("Lump Offset = %d", int(CurrentEntry->LumpOffset));

			if (ImGui::Button("Load Lump"))
				LoadLumpData(CurrentEntry);

			auto itr = LumpDB.find(CurrentEntry->Name);
			if (itr != LumpDB.end() && itr->second && itr->second->Visualize)
			{
				itr->second->Visualize(itr->second);
			}
		}

		ImGui::End();

	//	ImGui::ShowDemoWindow();

		rlImGuiEnd();
		
		EndDrawing();
	}
	if (DataBuffer != nullptr)
		UnloadFileData(DataBuffer);

	rlImGuiShutdown();

	ClearLumpData();
	// cleanup
	CloseWindow();
	return 0;
}