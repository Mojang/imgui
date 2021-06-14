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

# USE_NATIVE_IMGUI_INCLUDE_PATHS is set to true by default include direct path to imgui headers
# User can set it to false to create a custom way of including imgui to their project
option(USE_NATIVE_IMGUI_INCLUDE_PATHS "Set this to false if you want to define your own target_include_directories function for imgui." true)

if(USE_NATIVE_IMGUI_INCLUDE_PATHS)
  target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
endif()
