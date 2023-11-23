#include "lump_inspectors.h"
#include "imgui.h"
#include "raylib.h"

void VisualizeThings(WADData::ThingsLump* things)
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

void VisualizeVertexes(WADData::VertexesLump* vertecies)
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

void VisualizeLineDefs(WADData::LineDefLump* lineDefs)
{
    ImGui::TextUnformatted("Line Definitions");
    if (ImGui::BeginListBox("##LineDefs", ImVec2(-FLT_MIN, 10 * ImGui::GetTextLineHeightWithSpacing())))
    {
        int count = 0;
        for (const auto& line : lineDefs->Contents)
        {
            const char* text = TextFormat("X %d Y %d###LineDefs%d", line.Start, line.End, count);
            if (ImGui::Selectable(text))
            {
            }
            count++;
        }

        ImGui::EndListBox();
    }
}

void VisualizeSideDefs(WADData::SideDefLump* sideDefs)
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

void VisualizeSectors(WADData::SectorsLump* sectorDefs)
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
        lump->Visualize = [](WADData::Lump* lump) {VisualizeThings((WADData::ThingsLump*)lump); };
    else if (name == WADData::VERTEXES)
        lump->Visualize = [](WADData::Lump* lump) {VisualizeVertexes((WADData::VertexesLump*)lump); };
    else if (name == WADData::LINEDEFS)
        lump->Visualize = [](WADData::Lump* lump) {VisualizeLineDefs((WADData::LineDefLump*)lump); };
    else if (name == WADData::SIDEDEFS)
        lump->Visualize = [](WADData::Lump* lump) {VisualizeSideDefs((WADData::SideDefLump*)lump); };
	else if (name == WADData::SECTORS)
		lump->Visualize = [](WADData::Lump* lump) {VisualizeSectors((WADData::SectorsLump*)lump); };
}

