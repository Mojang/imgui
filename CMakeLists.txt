add_library(imgui STATIC)

target_sources(imgui
PRIVATE
  imgui.h
  imconfig.h
  imgui_demo.cpp
  imgui_draw.cpp
  imgui_internal.h
  imgui_widgets.cpp
  imgui.cpp
  "${CMAKE_CURRENT_SOURCE_DIR}/misc/natvis/imgui.natvis"
)
