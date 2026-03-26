project(ImGuizmo)

add_library(ImGuizmo STATIC 
    ImGuizmo.cpp 
    ImGuizmo.h
)

# ImGuizmo a besoin des headers d'ImGui pour compiler
target_link_libraries(ImGuizmo PUBLIC imgui) 

target_include_directories(ImGuizmo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})