message(STATUS "BUILDING PLUGIN_AUTOMASK")

file(GLOB SOURCE_FILES
    "*.h"
    "*.cpp"
)

add_library(automask STATIC
    ${SOURCE_FILES}
)

target_link_libraries(automask PUBLIC
    Qt6::Core
    Qt6::Multimedia
    Qt6::WebSockets
    Qt6::Sql
    PNG::PNG
)

target_include_directories(automask PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(automask PUBLIC
    ${RTCOM-EVENTLOGGER_CFLAGS_OTHER}
    ${OSSO_ABOOK_CFLAGS_OTHER}
    ${GTK_CFLAGS_OTHER}
)
