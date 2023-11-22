#include "lump_inspectors.h"
#include "imgui.h"
#include "raylib.h"

void VisualizeThings(WADData::ThingsLump* things)
{
    ImGui::TextUnformatted("Things");
    if (ImGui::BeginListBox("##Things", ImVec2(-FLT_MIN, 30 * ImGui::GetTextLineHeightWithSpacing())))
    {
        int count = 0;
        for (const auto& thing : things->Contents)
        {
            const char* text = TextFormat(" T %d X %d Y %d###Thing%d", thing.TypeId, thing.X, thing.Y, count);
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
    if (ImGui::BeginListBox("##Vertexes", ImVec2(-FLT_MIN, 30 * ImGui::GetTextLineHeightWithSpacing())))
    {
        int count = 0;
        for (const auto& vertex : vertecies->Contents)
        {
            const char* text = TextFormat(" X %d Y %d###Vertex%d", vertex.X, vertex.Y, count);
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
    if (ImGui::BeginListBox("##LineDefs", ImVec2(-FLT_MIN, 30 * ImGui::GetTextLineHeightWithSpacing())))
    {
        int count = 0;
        for (const auto& line : lineDefs->Contents)
        {
            const char* text = TextFormat(" X %d Y %d###LineDefs%d", line.Start, line.End, count);
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
}

