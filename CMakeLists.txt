cmake_minimum_required(VERSION 3.20)
project(AxionDigitaverse CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ================================================================
#  Build type default
# ================================================================
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# ================================================================
#  Auto-download ImGui + GLFW using CMake FetchContent
#  No manual downloads needed — cmake .. does it all.
# ================================================================
include(FetchContent)

# ── ImGui ─────────────────────────────────────────────────
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        v1.91.6
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(imgui)

# ── GLFW (window + OpenGL context) ────────────────────────
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.4
  GIT_SHALLOW    TRUE
)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# ================================================================
#  ImGui source files (core + backends we use)
# ================================================================
set(IMGUI_SOURCES
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

# ================================================================
#  Platform-specific OpenGL
# ================================================================
if(WIN32)
  set(OPENGL_LIBS opengl32)
elseif(APPLE)
  find_package(OpenGL REQUIRED)
  set(OPENGL_LIBS OpenGL::GL)
else()
  find_package(OpenGL REQUIRED)
  set(OPENGL_LIBS OpenGL::GL)
endif()

# ================================================================
#  Axion Digitaverse executable
# ================================================================
add_executable(AxionDigitaverse
  main_gui.cpp
  ${IMGUI_SOURCES}
)

target_include_directories(AxionDigitaverse PRIVATE
  ${CMAKE_SOURCE_DIR}
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(AxionDigitaverse PRIVATE
  glfw
  ${OPENGL_LIBS}
  $<$<PLATFORM_ID:Windows>:ws2_32>
  $<$<PLATFORM_ID:Linux>:pthread>
  $<$<PLATFORM_ID:Darwin>:pthread>
)

# ================================================================
#  Windows: hide console window, set app icon
# ================================================================
if(WIN32)
  target_link_options(AxionDigitaverse PRIVATE
    $<$<CONFIG:Release>:/SUBSYSTEM:WINDOWS>
    $<$<CONFIG:Release>:/ENTRY:mainCRTStartup>
  )
  # Manifest for DPI awareness
  target_sources(AxionDigitaverse PRIVATE
    ${CMAKE_SOURCE_DIR}/axion.manifest
  )
endif()

# ================================================================
#  Compiler flags
# ================================================================
target_compile_definitions(AxionDigitaverse PRIVATE
  NOMINMAX
  _CRT_SECURE_NO_WARNINGS
  $<$<CONFIG:Release>:NDEBUG>
)

if(MSVC)
  target_compile_options(AxionDigitaverse PRIVATE
    /W3 /MP /utf-8
    $<$<CONFIG:Release>:/O2>
  )
else()
  target_compile_options(AxionDigitaverse PRIVATE
    -Wall -Wextra
    $<$<CONFIG:Release>:-O2>
  )
endif()

# ================================================================
#  Install
# ================================================================
install(TARGETS AxionDigitaverse DESTINATION bin)

message(STATUS "Axion Digitaverse configured.")
message(STATUS "  Build type : ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ std    : ${CMAKE_CXX_STANDARD}")
message(STATUS "  ImGui      : will be fetched automatically")
message(STATUS "  GLFW       : will be fetched automatically")
