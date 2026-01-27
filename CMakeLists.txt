cmake_minimum_required(VERSION 3.5)
project(VraKtalEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------
# Output folders
# ---------------------------------------------------------
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

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
# imgui
# ---------------------------------------------------------
add_subdirectory(external/include/imgui)


# ---------------------------------------------------------
# imguizmo
# ---------------------------------------------------------
add_subdirectory(external/include/imguizmo)


# ---------------------------------------------------------
# glm
# ---------------------------------------------------------
add_subdirectory(external/include/glm)


# ---------------------------------------------------------
# tracy
# ---------------------------------------------------------
add_subdirectory(external/tracy)


# ---------------------------------------------------------
# bass
# ---------------------------------------------------------
add_library(bass SHARED IMPORTED)
set_target_properties(bass PROPERTIES
    IMPORTED_IMPLIB "${CMAKE_SOURCE_DIR}/external/lib/bass.lib"
    IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/external/lib/bass.dll"
)


# ---------------------------------------------------------
# engine Executable
# ---------------------------------------------------------
# Note: GLOB_RECURSE est pratique au début mais dangereux si tu ajoutes des fichiers hors IDE.
# Pense à relancer CMake quand tu ajoutes un fichier .cpp
file(GLOB_RECURSE ENGINE_SOURCES
    CONFIGURE_DEPENDS
    VraKtalEngine/*.cpp
)

add_library(VraKtalEngine STATIC ${ENGINE_SOURCES})

set_target_properties(VraKtalEngine PROPERTIES
    DEBUG_POSTFIX "_Debug"
)

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
    target_compile_options(VraKtalEngine PRIVATE /W3 /wd4267 /wd4834)
    # /wd4267 : conversion size_t -> uint32_t (très fréquent avec Vulkan vs std::vector)
    # /wd4834 : discard return value (nodiscard)
endif()

# ---------------------------------------------------------
# editor Executable
# ---------------------------------------------------------
file(GLOB_RECURSE EDITOR_SOURCES
    CONFIGURE_DEPENDS
    VraKtalEditor/*.cpp
)

add_executable(VraKtalEditor ${EDITOR_SOURCES})

target_link_libraries(VraKtalEditor PUBLIC VraKtalEngine)



# -----------------------------------
# Copier les DLLs
# -----------------------------------
add_custom_command(TARGET VraKtalEditor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/external/lib/glfw3.dll"
        "$<TARGET_FILE_DIR:VraKtalEditor>/glfw3.dll"
)

add_custom_command(TARGET VraKtalEditor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/external/lib/bass.dll"
    "$<TARGET_FILE_DIR:VraKtalEditor>/bass.dll"
)
