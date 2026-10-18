include(FetchContent)

set(SFML_VERSION 3.1.0)
set(IMGUI_VERSION 1.91.9b)

option(LINK_DEPS_STATIC CACHE ON)

# Don't build shared libs if we want to link to deps statically
if (LINK_DEPS_STATIC)
    set(BUILD_SHARED_LIBS OFF)
else ()
    set(BUILD_SHARED_LIBS ON)
endif ()

# SFML
FetchContent_Declare(
        SFML
        URL "https://github.com/SFML/SFML/archive/${SFML_VERSION}.zip"
        DOWNLOAD_EXTRACT_TIMESTAMP
)

# switch off the (optional) audio and network modules
option(SFML_BUILD_AUDIO "Build SFML audio module" OFF)
option(SFML_BUILD_NETWORK "Build SFML network module" OFF)
FetchContent_MakeAvailable(sfml)

# IMGUI
FetchContent_Declare(
        imgui
        URL "https://github.com/ocornut/imgui/archive/v${IMGUI_VERSION}.zip"
        DOWNLOAD_EXTRACT_TIMESTAMP
)
FetchContent_MakeAvailable(imgui)

# IMGUI-SFML
FetchContent_Declare(
        imgui-sfml
        GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
        GIT_TAG v3.0
)
set(IMGUI_DIR ${imgui_SOURCE_DIR})

option(IMGUI_SFML_FIND_SFML "Use find_package to find SFML" OFF)
option(IMGUI_SFML_IMGUI_DEMO "Build imgui_demo.cpp" ON)

FetchContent_MakeAvailable(imgui-sfml)

# ASIO
FetchContent_Declare(asiocmake
        GIT_REPOSITORY "https://github.com/OlivierLDff/asio.cmake"
        GIT_TAG "main"
)
FetchContent_MakeAvailable(asiocmake)