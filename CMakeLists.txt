cmake_minimum_required (VERSION 3.29)

project(gateway-config
  LANGUAGES CXX
  VERSION 1.0.0
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Fetch FTXUI --------------------------------------------------------------
include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED OFF)
set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/arthursonzogni/ftxui.git
  GIT_TAG        v6.0.0
  GIT_PROGRESS   TRUE
  GIT_SHALLOW    TRUE
)

FetchContent_Declare(ftxui-ip-input
  GIT_REPOSITORY https://github.com/mingsheng13/ip-input-ftxui.git
  GIT_TAG main
  GIT_PROGRESS TRUE
  GIT_SHALLOW  TRUE
)
FetchContent_Declare(json 
  URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
  GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(ftxui)
FetchContent_MakeAvailable(ftxui-ip-input)
FetchContent_MakeAvailable(json)


# ------------------------------------------------------------------------------

add_executable(gateway-config src/main.cpp
        src/scrollerbase.h
        src/util.h
        src/components.h
        src/elements.h
        src/networksetting.cpp
        src/networksetting.h
        src/gatewaysetting.cpp
        src/gatewaysetting.h
        src/rcsusetting.cpp
        src/rcsusetting.h
)
target_include_directories(gateway-config PRIVATE src)

target_link_libraries(gateway-config
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component  # Not needed for this example.
  PRIVATE ftxui-ip-input
  PRIVATE nlohmann_json::nlohmann_json
)
