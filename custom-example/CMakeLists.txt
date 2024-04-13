add_compile_definitions(
    CUSTOMHEADER="CustomPlugin.h"
    CUSTOMCLASS=CustomPlugin
)

list(APPEND QGC_RESOURCES ${CMAKE_SOURCE_DIR}/custom/custom.qrc)

add_subdirectory(src)