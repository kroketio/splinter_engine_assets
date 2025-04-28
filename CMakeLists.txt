cmake_minimum_required(VERSION 3.16)
project(asset_browser
  VERSION "1.0.0"
  DESCRIPTION "asset browser"
  LANGUAGES CXX
)
message(STATUS "Initiating compile using CMake ${CMAKE_VERSION}")

# add cmake/ directory as cmake module path
list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_SOURCE_DIR}/cmake")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
        "Default build type: Debug" FORCE)
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_CXX_STANDARD 20)
option(BUILD_SERVER "" ON)
option(BUILD_CLIENT "" OFF)
option(PLUGIN_AUTOMASK "" OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include(FindCcache)

find_package(ZLIB REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(PNG REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Concurrent Multimedia WebSockets Sql HttpServer OpenGL QuickWidgets Gui Quick Qml)
#pkg_check_modules(LIBTORRENT REQUIRED libtorrent)
#pkg_check_modules(LIBTORRENT_RASTERBAR REQUIRED libtorrent-rasterbar)

add_subdirectory(shared)
add_subdirectory(client)

if(BUILD_SERVER)
  add_subdirectory(server)
endif()
