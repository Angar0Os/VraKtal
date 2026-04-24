#include "../../../include/windows/others/meshPlot.h"
#include "../../../include/imGuiWindows.h"

#ifndef IMPLOT_DISABLE_OBSOLETE_FUNCTIONS
#define IMPLOT_DISABLE_OBSOLETE_FUNCTIONS
#endif


#include <imPlot3D/implot3d.h>
#include <imPlot3D/implot3d_internal.h>

#define CHECKBOX_FLAG(flags, flag , title) ImGui::CheckboxFlags(title, (unsigned int*)&flags, flag)


MeshPlot::MeshPlot(ImGuiWindows* _windowManager)
{
    m_windowManager = _windowManager;
}

MeshPlot::~MeshPlot()
{
}

void MeshPlot::Draw(graphics::resources::Mesh* _mesh)
{
    // Choose line color
    static ImVec4 line_color = ImVec4(0.5f, 0.5f, 0.2f, 0.6f);
    ImGui::ColorEdit4("Line Color##Mesh", (float*)&line_color , ImGuiColorEditFlags_NoInputs);
    ImGui::SameLine();
    // Choose fill color
    static ImVec4 fill_color = ImVec4(0.8f, 0.8f, 0.2f, 0.6f);
    ImGui::ColorEdit4("Fill Color##Mesh", (float*)&fill_color , ImGuiColorEditFlags_NoInputs);
    ImGui::SameLine();
    // Choose marker color
    static ImVec4 marker_color = ImVec4(0.5f, 0.5f, 0.2f, 0.6f);
    ImGui::ColorEdit4("Marker Color##Mesh", (float*)&marker_color, ImGuiColorEditFlags_NoInputs);

    // Mesh flags
    static ImPlot3DMeshFlags flags = ImPlot3DMeshFlags_NoMarkers;
    CHECKBOX_FLAG(flags, ImPlot3DMeshFlags_NoLines  ,"Lines");
    ImGui::SameLine();
    CHECKBOX_FLAG(flags, ImPlot3DMeshFlags_NoFill   ,"Fill");
    ImGui::SameLine();
    CHECKBOX_FLAG(flags, ImPlot3DMeshFlags_NoMarkers,"Markers");

    if (ImPlot3D::BeginPlot("Preview")) {
        ImPlot3D::SetupAxesLimits(-1, 1, -1, 1, -1, 1);

        ImPlot3DSpec spec;
        spec.Flags = flags;
        spec.Stride = (int)sizeof(ImPlot3DPoint);
        // Set fill style
        spec.FillColor = fill_color;
        // Set line style
        spec.LineColor = line_color;
        // Set marker style
        spec.Marker = ImPlot3DMarker_Square;
        spec.MarkerSize = 3.0f;
        spec.MarkerLineColor = marker_color;
        spec.MarkerFillColor = marker_color;
        
        ImPlot3DSpec meshSpec = spec;
        meshSpec.Stride = sizeof(graphics::resources::Vertex);

        ImPlot3D::PlotMesh(
            "",
            &_mesh->vertices[0].position.x,
            &_mesh->vertices[0].position.y,
            &_mesh->vertices[0].position.z,
            _mesh->indices.data(),
            static_cast<int>(_mesh->GetVertexCount()),
            static_cast<int>(_mesh->GetIndexCount()),
            meshSpec
        );

        ImPlot3D::EndPlot();
    }
}
