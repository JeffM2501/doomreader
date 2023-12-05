#include "lump_inspectors.h"
#include "imgui.h"
#include "raylib.h"

#include "doom_map.h"

void VisualizeThings(WADData::ThingsLump* things, WADFile::LevelMap* map)
{
    ImGui::TextUnformatted("Things");
    if (ImGui::BeginListBox("##Things", ImVec2(-FLT_MIN, 10 * ImGui::GetTextLineHeightWithSpacing())))
    {
        int count = 0;
        for (const auto& thing : things->Contents)
        {
            const char* text = TextFormat("T %d X %d Y %d###Thing%d", thing.TypeId, thing.X, thing.Y, count);
            if (ImGui::Selectable(text))
            {
            }
            count++;
        }

        ImGui::EndListBox();
    }
}

void VisualizeVertexes(WADData::VertexesLump* vertecies, WADFile::LevelMap* map)
{
    ImGui::TextUnformatted("Vertexes");
    if (ImGui::BeginListBox("##Vertexes", ImVec2(-FLT_MIN, 10 * ImGui::GetTextLineHeightWithSpacing())))
    {
        int count = 0;
        for (const auto& vertex : vertecies->Contents)
        {
            const char* text = TextFormat("X %d Y %d###Vertex%d", vertex.X, vertex.Y, count);
            if (ImGui::Selectable(text))
            {
            }
            count++;
        }

        ImGui::EndListBox();
    }
}

static int CurrentLine = -1;

void VisualizeLineDefs(WADData::LineDefLump* lineDefs, WADFile::LevelMap* map)
{
    ImGui::TextUnformatted("Line Definitions");
    if (ImGui::BeginListBox("##LineDefs", ImVec2(-FLT_MIN, 10 * ImGui::GetTextLineHeightWithSpacing())))
    {
        int count = 0;
        for (const auto& line : lineDefs->Contents)
        {
            const char* text = TextFormat("Index %d, X %d Y %d###LineDefs%d", count, line.Start, line.End, count);

            bool selected = CurrentLine == count;
            if (ImGui::Selectable(text))
            {
                CurrentLine = count;
            }
            count++;
        }

        ImGui::EndListBox();

        if (CurrentLine >= 0)
        {
            const auto& line = lineDefs->Contents[CurrentLine];

            ImGui::Text("SP %d, EP %d", line.Start, line.End);
            ImGui::Text("Front %d, Back %d", line.FrontSideDef, line.BackSideDef);
            ImGui::Text("Flats %d, Type %d, SectorTag", line.Flags, line.SpecialType, line.SectorTag);
        }
    }
}

void VisualizeSideDefs(WADData::SideDefLump* sideDefs, WADFile::LevelMap* map)
{
    ImGui::TextUnformatted("Side Definitions");
    if (ImGui::BeginListBox("##SideDefs", ImVec2(-FLT_MIN, 10 * ImGui::GetTextLineHeightWithSpacing())))
    {
        int count = 0;
        for (const auto& side : sideDefs->Contents)
        {
            const char* text = TextFormat("SectorId %d T %s M %s B %s ##Side%d", side.SectorId, side.TopTexture.c_str(), side.MidTexture.c_str(), side.LowerTexture.c_str(), count);
            if (ImGui::Selectable(text))
            {
            }
            count++;
        }

        ImGui::EndListBox();
    }
}

void VisualizeSectors(WADData::SectorsLump* sectorDefs, WADFile::LevelMap* map)
{
	ImGui::TextUnformatted("Side Definitions");
	if (ImGui::BeginListBox("##SideDefs", ImVec2(-FLT_MIN, 10 * ImGui::GetTextLineHeightWithSpacing())))
	{
		int count = 0;
		for (const auto& sector : sectorDefs->Contents)
		{
			const char* text = TextFormat("S %d, F %d C %d FT %s CT %s###Sector%d", count, sector.FloorHeight, sector.CeilingHeight, sector.FloorTexture.c_str(), sector.CeilingTexture.c_str(), count);
			if (ImGui::Selectable(text))
			{
			}
			count++;
		}

		ImGui::EndListBox();
	}
}

void SetupLumpInspector(const std::string& name, WADData::Lump* lump)
{
    if (!lump)
        return;

    if (name == WADData::THINGS)
        lump->Visualize = [](WADData::Lump* lump, void* map) {VisualizeThings((WADData::ThingsLump*)lump, (WADFile::LevelMap*)map); };
    else if (name == WADData::VERTEXES)
        lump->Visualize = [](WADData::Lump* lump, void* map) {VisualizeVertexes((WADData::VertexesLump*)lump, (WADFile::LevelMap*)map); };
    else if (name == WADData::LINEDEFS)
        lump->Visualize = [](WADData::Lump* lump, void* map) {VisualizeLineDefs((WADData::LineDefLump*)lump, (WADFile::LevelMap*)map); };
    else if (name == WADData::SIDEDEFS)
        lump->Visualize = [](WADData::Lump* lump, void* map) {VisualizeSideDefs((WADData::SideDefLump*)lump, (WADFile::LevelMap*)map); };
	else if (name == WADData::SECTORS)
		lump->Visualize = [](WADData::Lump* lump, void* map) {VisualizeSectors((WADData::SectorsLump*)lump, (WADFile::LevelMap*)map); };
}

