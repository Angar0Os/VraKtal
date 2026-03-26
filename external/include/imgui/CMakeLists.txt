cmake_minimum_required(VERSION 3.10)
project(ImGui LANGUAGES CXX)

message(STATUS "GLFW include path: ${CMAKE_SOURCE_DIR}/external/include/GLFW")
# ---------------------------------------------------------
# Core ImGui
# ---------------------------------------------------------
file(GLOB IMGUI_CORE
    imgui.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
)

# ---------------------------------------------------------
# Backends
# ---------------------------------------------------------
file(GLOB IMGUI_BACKENDS
    imgui_impl_glfw.cpp
    imgui_impl_vulkan.cpp
)


# ---------------------------------------------------------
# Créer la lib
# ---------------------------------------------------------
add_library(imgui STATIC 
	${IMGUI_CORE}
	${IMGUI_BACKENDS}
)

target_include_directories(imgui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)


# ---------------------------------------------------------
# Dépendances
# ---------------------------------------------------------

find_package(Vulkan REQUIRED)
target_link_libraries(imgui PUBLIC glfw Vulkan::Vulkan)



