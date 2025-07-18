set(CMAKE_CXX_STANDARD 20)

file(GLOB SOURCE_FILES
    "*.h"
    "*.cpp"
    "lib/*.h"
    "lib/*.cpp"
    "lib/network/*.h"
    "lib/network/*.cpp"
    "lib/network/torrent/*.h"
    "lib/network/torrent/*.cpp"
    "models/*.h"
    "models/*.cpp"
)

#set(shared_resource_files
#    "assets/brickwall.tres"
#    "assets/cobblestone.tres"
#    "assets/concrete.tres"
#    "assets/default.tres"
#    "assets/grass_leafs.tres"
#    "assets/grass.tres"
#    "assets/metal_sheet.tres"
#    "assets/planks.tres"
#    "assets/plaster_damaged.tres"
#    "assets/stone.tres"
#    "assets/tiles.tres"
#)

qt_add_resources(GENERATED_RESOURCES assets.qrc)
add_library(shared STATIC
    ${SOURCE_FILES}
    ${GENERATED_RESOURCES}
)

target_link_libraries(shared PUBLIC
    Qt6::Core
    Qt6::Multimedia
    Qt6::WebSockets
    Qt6::Sql
    PNG::PNG
    ZLIB::ZLIB
)

target_include_directories(shared PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(shared PUBLIC
    ${RTCOM-EVENTLOGGER_CFLAGS_OTHER}
    ${OSSO_ABOOK_CFLAGS_OTHER}
    ${GTK_CFLAGS_OTHER}
)

if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    target_compile_definitions(shared PUBLIC GENERATE_VTF=1)
endif()
