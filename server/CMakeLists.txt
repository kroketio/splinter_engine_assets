file(GLOB SERVER_SOURCE_FILES
    "*.h"
    "*.cpp"
    "external/stb/*.h"
    "external/stb/*.cpp"
    "lib/vtf/*.h"
    "lib/vtf/*.cpp"
    "lib/vtf/common/*.h"
    "lib/vtf/common/*.cpp"
)

set(VTFLIB_STATIC OFF CACHE STRING "VTFLIB_STATIC")
add_subdirectory(external/VTFLib)

add_executable(server main.cpp
    ${SERVER_SOURCE_FILES}
    ${RESOURCE_FILES}
)

target_include_directories(server PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/external/VTFLib/VTFLib
        external)

target_link_libraries(server PUBLIC
    Qt6::Core
    Qt6::Multimedia
    Qt6::WebSockets
    Qt6::Sql
    Qt6::HttpServer
    Qt6::Concurrent
    PNG::PNG
    shared
    vtflib
)

if(UNIX AND NOT APPLE AND PLUGIN_AUTOMASK)
    add_subdirectory(plugin_automask)
    target_compile_definitions(server PRIVATE PLUGIN_AUTOMASK=1)
endif()

# generate globals.h (path to custom Qt plugins dir)
#configure_file(
#        ${CMAKE_SOURCE_DIR}/cmake/globals.h.cmake
#        ${CMAKE_BINARY_DIR}/example/globals.h
#        @ONLY # only @VARIABLE@ placeholders
#)

set_target_properties(server PROPERTIES
        WIN32_EXECUTABLE TRUE
        MACOSX_BUNDLE TRUE
)

install(TARGETS server
        RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
        BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
        LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)

install(TARGETS server
        DESTINATION ${CMAKE_INSTALL_PREFIX}
)

set_target_properties(server PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        LINK_FLAGS_RELEASE -s
)
