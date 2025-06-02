cmake_minimum_required(VERSION 3.10.0)
project(MapCompiler VERSION 0.1.0 LANGUAGES C CXX)

add_subdirectory(libs/raylib)
add_subdirectory(libs/glm)
add_subdirectory(libs/physfs)

set(CMAKE_CXX_STANDARD 17)

# raylib settings
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES OFF CACHE BOOL "" FORCE) # or games
set(PHYSFS_TARGETNAME_UNINSTALL OFF CACHE BOOL "Name of 'uninstall' build target" FORCE) # don't build physfs uninstall target

add_executable(MapCompiler
    src/main.cpp
    src/FS/FS.cpp
    src/MapFormat/Face.cpp
    src/MapFormat/Brush.cpp
    src/MapFormat/Patch.cpp
    src/MapFormat/Entity.cpp
    src/MapFormat/Map.cpp
    src/MapFormat/Lexer.cpp
    src/MapFormat/Parser.cpp
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/raylib"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/glm"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/physfs/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

include(CTest)
enable_testing()

target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
target_link_libraries(${PROJECT_NAME} PRIVATE glm)
target_link_libraries(${PROJECT_NAME} PRIVATE physfs-static)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
