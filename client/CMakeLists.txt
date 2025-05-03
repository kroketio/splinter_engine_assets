set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

file(GLOB SOURCE_FILES
        "*.h"
        "*.cpp"
        "engine/gl/*.h"
        "engine/gl/*.cpp"
        "engine/core/*.h"
        "engine/core/*.cpp"
        "lib/*.h"
        "lib/*.cpp"
        "lib/qt/*.h"
        "lib/qt/*.cpp"
        "lib/vmfpp/*.h"
        "lib/vmfpp/*.cpp"
        "lib/vmfpp/detail/*.h"
        "lib/vmfpp/detail/*.cpp"
        "lib/vmfpp/structs/*.h"
        "lib/vmfpp/structs/*.cpp"
)

set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INJECT_DEBUG_POSTFIX OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ZLIB OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL_PDB OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_VIEW OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "" FORCE)

add_subdirectory(engine/assimp)

qt_add_resources(RESOURCES engine/engine.qrc lib/qt/theme.qrc)
add_executable(client ${SOURCE_FILES} ${RESOURCES})


target_compile_options(client PRIVATE -Wno-deprecated-declarations)
target_include_directories(client PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(client PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED YES)

target_link_libraries(client PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::Widgets
        Qt6::QuickWidgets
        Qt6::OpenGL
        Qt6::WebSockets
        Qt6::Sql
        PNG::PNG
        ZLIB::ZLIB
        shared
        assimp::assimp
)