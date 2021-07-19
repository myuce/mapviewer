cmake_minimum_required(VERSION 3.0.0)
project(mapviewer VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(mapviewer
  main.cpp
  rlImGui/rlImGui.cpp
  imgui/imgui.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_tables.cpp
  imgui/imgui_widgets.cpp
  imgui/imgui_demo.cpp
  rlExtrasC/RLGeoTools.c
)
target_link_libraries(mapviewer
  ${CMAKE_SOURCE_DIR}/lib/libraylib.a
  opengl32
  gdi32
  winmm
  imm32
  oleaut32
  ole32
)
target_include_directories(mapviewer PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/rlExtrasC
  ${CMAKE_SOURCE_DIR}/rlExtrasCPP
  ${CMAKE_SOURCE_DIR}/imgui
  ${CMAKE_SOURCE_DIR}/rlImgui
)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wpedantic")
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
