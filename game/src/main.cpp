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
	WADData::VertexesLump* verts = (WADData::VertexesLump*)LumpDB[WADData::VERTEXES];
}

int main ()
{
	// set up the window
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(1280, 800, "Hello DOOM!");

	rlImGuiSetup(true);

	ReadLevelWad();

	Camera2D cam = { 0 };
	cam.zoom = 1;
	
	// game loop
	while (!WindowShouldClose())
	{
		// drawing
		BeginDrawing();
		ClearBackground(BLACK);

		cam.offset = Vector2Scale(Vector2{ (float)GetScreenWidth(), (float)GetScreenHeight() }, 0.5f);

		BeginMode2D(cam);

		DrawLevelLines();

		EndMode2D();

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

			auto itr = LumpDB.find(CurrentEntry);
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