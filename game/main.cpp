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
#include "reader.h"
#include "rlImGui.h"
#include "imgui.h"

std::vector<WADData::DirectoryEntry> Entries;
std::unordered_map<const WADData::DirectoryEntry*, WADData::Lump*> LumpDB;

uint8_t* DataBuffer = nullptr;

const WADData::DirectoryEntry* CurrentEntry = nullptr;

void ReadLevelWad()
{
	int size = 0;
	DataBuffer = LoadFileData("resources/E1M1.wad", &size);

	Entries = WADReader::ReadDirectoryEntries(DataBuffer);
}

void LoadLumpData(const WADData::DirectoryEntry* entry)
{
	if (!entry|| LumpDB.find(entry) != LumpDB.end())
		return;

	WADData::Lump* lump = WADData::GetLump(entry->Name);
	if (!lump)
		return;

	lump->Parse(DataBuffer, entry->LumpOffset, entry->LumpSize);
	LumpDB.insert_or_assign(entry, lump);
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

int main ()
{
	// set up the window
	InitWindow(1280, 800, "Hello DOOM!");

	rlImGuiSetup(true);

	ReadLevelWad();
	
	// game loop
	while (!WindowShouldClose())
	{
		// drawing
		BeginDrawing();
		ClearBackground(BLACK);

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
		}

		ImGui::End();

		ImGui::ShowDemoWindow();

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