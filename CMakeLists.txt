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
#  Axion IDE Library
# ================================================================
add_library(axion_ide STATIC
  axion_ide_impl.cpp
  ${IMGUI_SOURCES}
)

target_include_directories(axion_ide PUBLIC
  ${CMAKE_SOURCE_DIR}
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(axion_ide PUBLIC
  axion_avm_v2
  glfw
  ${OPENGL_LIBS}
)

target_compile_definitions(axion_ide PRIVATE
  NOMINMAX
  _CRT_SECURE_NO_WARNINGS
)

if(MSVC)
  target_compile_options(axion_ide PRIVATE /W3 /MP /utf-8)
else()
  target_compile_options(axion_ide PRIVATE -Wall -Wextra)
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
  axion_avm_v2
  axion_ide
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
#  Advanced AVM v2.0 (for Phase 2)
# ================================================================
add_library(axion_avm_v2 STATIC
  avm_v2.cpp
  avm_python.cpp
)

target_include_directories(axion_avm_v2 PUBLIC
  ${CMAKE_SOURCE_DIR}
)

target_compile_definitions(axion_avm_v2 PRIVATE
  NOMINMAX
  _CRT_SECURE_NO_WARNINGS
)

if(MSVC)
  target_compile_options(axion_avm_v2 PRIVATE /W3 /MP /utf-8)
else()
  target_compile_options(axion_avm_v2 PRIVATE -Wall -Wextra)
endif()

# ================================================================
#  AVM Test executable
# ================================================================
add_executable(test_avm_v2
  tests/test_avm_v2.cpp
)

target_include_directories(test_avm_v2 PRIVATE
  ${CMAKE_SOURCE_DIR}
)

target_link_libraries(test_avm_v2 PRIVATE
  axion_avm_v2
)

target_compile_definitions(test_avm_v2 PRIVATE
  NOMINMAX
  _CRT_SECURE_NO_WARNINGS
)

if(MSVC)
  target_compile_options(test_avm_v2 PRIVATE /W3 /MP /utf-8)
else()
  target_compile_options(test_avm_v2 PRIVATE -Wall -Wextra)
endif()

# ================================================================
#  Python Runtime Test executable
# ================================================================
add_executable(test_python_runtime
  tests/test_python_runtime.cpp
)

target_include_directories(test_python_runtime PRIVATE
  ${CMAKE_SOURCE_DIR}
)

target_link_libraries(test_python_runtime PRIVATE
  axion_avm_v2
)

target_compile_definitions(test_python_runtime PRIVATE
  NOMINMAX
  _CRT_SECURE_NO_WARNINGS
)

if(MSVC)
  target_compile_options(test_python_runtime PRIVATE /W3 /MP /utf-8)
else()
  target_compile_options(test_python_runtime PRIVATE -Wall -Wextra)
endif()

# ================================================================
#  Axion IDE Test executable
# ================================================================
add_executable(test_axion_ide
  tests/test_axion_ide.cpp
)

target_include_directories(test_axion_ide PRIVATE
  ${CMAKE_SOURCE_DIR}
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(test_axion_ide PRIVATE
  axion_avm_v2
  axion_ide
)

target_compile_definitions(test_axion_ide PRIVATE
  NOMINMAX
  _CRT_SECURE_NO_WARNINGS
)

if(MSVC)
  target_compile_options(test_axion_ide PRIVATE /W3 /MP /utf-8)
else()
  target_compile_options(test_axion_ide PRIVATE -Wall -Wextra)
endif()

# ================================================================
#  CLI Node executable (headless server)
# ================================================================
add_executable(axion_node
  main.cpp
)

target_link_libraries(axion_node PRIVATE
  axion_avm_v2
)

target_include_directories(axion_node PRIVATE
  ${CMAKE_SOURCE_DIR}
)

target_link_libraries(axion_node PRIVATE
  $<$<PLATFORM_ID:Windows>:ws2_32>
  $<$<PLATFORM_ID:Linux>:pthread>
  $<$<PLATFORM_ID:Darwin>:pthread>
)

target_compile_definitions(axion_node PRIVATE
  NOMINMAX
  _CRT_SECURE_NO_WARNINGS
  $<$<CONFIG:Release>:NDEBUG>
)

if(MSVC)
  target_compile_options(axion_node PRIVATE
    /W3 /MP /utf-8
    $<$<CONFIG:Release>:/O2>
  )
else()
  target_compile_options(axion_node PRIVATE
    -Wall -Wextra
    $<$<CONFIG:Release>:-O2>
  )
endif()

# ================================================================
#  Standalone IDE Launcher (MAIN APPLICATION)
# ================================================================
add_executable(AxionIDE
  ide_launcher.cpp
  ${IMGUI_SOURCES}
)

target_include_directories(AxionIDE PRIVATE
  ${CMAKE_SOURCE_DIR}
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(AxionIDE PRIVATE
  glfw
  ${OPENGL_LIBS}
  axion_avm_v2
  axion_ide
  $<$<PLATFORM_ID:Windows>:ws2_32>
  $<$<PLATFORM_ID:Linux>:pthread>
  $<$<PLATFORM_ID:Darwin>:pthread>
)

if(WIN32)
  target_link_options(AxionIDE PRIVATE
    $<$<CONFIG:Release>:/SUBSYSTEM:WINDOWS>
    $<$<CONFIG:Release>:/ENTRY:mainCRTStartup>
  )
endif()

target_compile_definitions(AxionIDE PRIVATE
  NOMINMAX
  _CRT_SECURE_NO_WARNINGS
  $<$<CONFIG:Release>:NDEBUG>
)

if(MSVC)
  target_compile_options(AxionIDE PRIVATE
    /W3 /MP /utf-8
    $<$<CONFIG:Release>:/O2>
  )
else()
  target_compile_options(AxionIDE PRIVATE
    -Wall -Wextra
    $<$<CONFIG:Release>:-O2>
  )
endif()

# ================================================================
#  Install targets
# ================================================================
install(TARGETS AxionDigitaverse axion_node
  RUNTIME DESTINATION bin
  BUNDLE DESTINATION Applications
)

# Install helper scripts
install(FILES 
  scripts/run-node.sh
  scripts/run-node.bat
  scripts/seed-node.sh
  scripts/seed-node.bat
  DESTINATION bin
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)

# Install documentation
install(FILES 
  README.md
  DEPLOYMENT.md
  NODE_SETUP.md
  DESTINATION docs
)

# Install data directory template
install(DIRECTORY axion_data
  DESTINATION .
  OPTIONAL
)

# ================================================================
#  CPack packaging
# ================================================================
include(CPack)

set(CPACK_PACKAGE_NAME "AxionDigitaverse")
set(CPACK_PACKAGE_VERSION "2.0.0")
set(CPACK_PACKAGE_VENDOR "Axion Foundation")
set(CPACK_PACKAGE_DESCRIPTION "Blockchain node and desktop application for Axion Digitaverse")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A full-featured blockchain platform with GUI and P2P networking")
set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/axion-logo.ico")

# Windows installer
if(WIN32)
  set(CPACK_GENERATOR NSIS;ZIP)
  set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/axion-logo.ico")
  set(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}/axion-logo.ico")
  set(CPACK_NSIS_INSTALLED_ICON_NAME "bin/AxionDigitaverse.exe")
  set(CPACK_NSIS_DISPLAY_NAME "Axion Digitaverse v2.0")
  set(CPACK_NSIS_URL_INFO_ABOUT "https://axiondigitaverse.com")
  set(CPACK_NSIS_CONTACT "support@axiondigitaverse.com")
  set(CPACK_NSIS_HELP_LINK "https://docs.axiondigitaverse.com")
endif()

# macOS installer
if(APPLE)
  set(CPACK_GENERATOR DragNDrop;ZIP)
  set(CPACK_DMG_VOLUME_NAME "AxionDigitaverse")
  set(CPACK_DMG_FORMAT UDZO)
endif()

# Linux packages
if(UNIX AND NOT APPLE)
  set(CPACK_GENERATOR DEB;RPM;TGZ)
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "libgl1-mesa-glx, libxrandr2")
  set(CPACK_RPM_PACKAGE_REQUIRES "mesa-libGL, libXrandr")
endif()

# Set common CPack variables
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_FILE_NAME "AxionDigitaverse-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}")

# ================================================================
#  Build targets status
# ================================================================
message(STATUS "")
message(STATUS "Axion Digitaverse v2.0 configured successfully")
message(STATUS "═════════════════════════════════════════════════")
message(STATUS "  Executables : AxionDigitaverse (GUI)")
message(STATUS "              : axion_node (CLI headless)")
message(STATUS "  Build type  : ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ std     : ${CMAKE_CXX_STANDARD}")
message(STATUS "  Generator   : Ninja (specified)")
message(STATUS "")
message(STATUS "Build commands:")
message(STATUS "  mkdir build && cd build")
message(STATUS "  cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release")
message(STATUS "  cmake --build . --config Release")
message(STATUS "  cpack -G ZIP  (or NSIS on Windows)")
message(STATUS "")
