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
        "lib/vmfpp/*.h"
        "lib/vmfpp/*.cpp"
        "lib/vmfpp/detail/*.h"
        "lib/vmfpp/detail/*.cpp"
        "lib/vmfpp/structs/*.h"
        "lib/vmfpp/structs/*.cpp"
)

add_executable(client ${SOURCE_FILES})
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
)