cmake_minimum_required(VERSION 4.0)
project(VraKtalEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------
# Output folders
# ---------------------------------------------------------
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/)

# ---------------------------------------------------------
# Vulkan
# ---------------------------------------------------------
find_package(Vulkan REQUIRED)


# ---------------------------------------------------------
# glfw
# ---------------------------------------------------------
add_library(glfw SHARED IMPORTED)
set_target_properties(glfw PROPERTIES
    IMPORTED_IMPLIB "${CMAKE_SOURCE_DIR}/external/lib/glfw3.lib"
    IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/external/lib/glfw3.dll"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/external/include"
)

# ---------------------------------------------------------
# bass
# ---------------------------------------------------------
add_library(bass SHARED IMPORTED)
set_target_properties(bass PROPERTIES
    IMPORTED_IMPLIB "${CMAKE_SOURCE_DIR}/external/lib/bass.lib"
    IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/external/lib/bass.dll"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/external/include"
)

# ---------------------------------------------------------
# imgui, imguizmo, glm, tracy
# ---------------------------------------------------------
add_subdirectory(external/include/imgui)
add_subdirectory(external/include/imguizmo)
add_subdirectory(external/include/glm)
add_subdirectory(external/tracy)

# ---------------------------------------------------------
# engine Executable
# ---------------------------------------------------------
# Note: GLOB_RECURSE est pratique au d but mais dangereux si tu ajoutes des fichiers hors IDE.
# Pense   relancer CMake quand tu ajoutes un fichier .cpp
file(GLOB_RECURSE ENGINE_SOURCES
    CONFIGURE_DEPENDS
    VraKtalEngine/*.cpp
)

add_library(VraKtalEngine STATIC ${ENGINE_SOURCES})

target_compile_definitions(VraKtalEngine PUBLIC 
    VK_USE_PLATFORM_WIN32_KHR
    NOMINMAX
    WIN32_LEAN_AND_MEAN
)

# OPTIMISATION : Precompiled Headers
target_precompile_headers(VraKtalEngine PRIVATE
    <vector>
    <string>
    <memory>
    "${CMAKE_SOURCE_DIR}/external/include/glm/glm.hpp"
)

#set_target_properties(VraKtalEngine PROPERTIES
#    DEBUG_POSTFIX "_Debug"
#)


target_include_directories(VraKtalEngine PUBLIC
    ${CMAKE_SOURCE_DIR}/VraKtalEngine/include
    ${CMAKE_SOURCE_DIR}/VraKtalEngine/src
    ${CMAKE_SOURCE_DIR}/external/include # Pour STB, TinyObj, etc.
)

target_link_libraries(VraKtalEngine PUBLIC
    glfw
    imgui
    ImGuizmo
    Vulkan::Vulkan
    glm
    TracyClient
)

if(MSVC)
    target_compile_options(VraKtalEngine PRIVATE /W3 /wd4267 /wd4834 /MP)
    # /wd4267 : conversion size_t -> uint32_t (tr s fr quent avec Vulkan vs std::vector)
    # /wd4834 : discard return value (nodiscard)
endif()

# ---------------------------------------------------------
# editor Executable
# ---------------------------------------------------------
file(GLOB_RECURSE EDITOR_SOURCES
    CONFIGURE_DEPENDS
    VraKtalEditor/*.cpp
)

add_compile_definitions(
    $<$<CONFIG:Debug>:_DEBUG
    $<$<CONFIG:Debug>:_CONSOLE 
    $<$<CONFIG:Debug>:VRAKTAL_EDITOR)
    #$<$<CONFIG:Release>:_RELEASE)



add_executable(VraKtalEditor ${EDITOR_SOURCES})
add_dependencies(VraKtalEditor VraKtalEngine)

target_include_directories(VraKtalEditor PUBLIC
    ${CMAKE_SOURCE_DIR}/VraKtalEditor/include
    ${CMAKE_SOURCE_DIR}/VraKtalEditor/src
)

set_target_properties(VraKtalEditor PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:VraKtalEditor>"
)

target_link_libraries(VraKtalEditor PUBLIC 
    VraKtalEngine
    imgui
)


# -----------------------------------
# Synchronisation Intelligente
# -----------------------------------
set(EDITOR_BIN_DIR "$<TARGET_FILE_DIR:VraKtalEditor>")

add_custom_command(TARGET VraKtalEditor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory 
        "${EDITOR_BIN_DIR}/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/external/lib/glfw3.dll"
        "${EDITOR_BIN_DIR}/glfw3.dll"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/external/lib/bass.dll"
        "${EDITOR_BIN_DIR}/bass.dll"
      COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/bin/assets"
        "${EDITOR_BIN_DIR}/assets"

    COMMENT "Synchronisation des DLLs et assets vers le dossier de build..."
)


