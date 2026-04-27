
project(WoTApp)

SET(SOURCE_FILES
    src/AppMain.cpp
    )

add_library(${PROJECT_NAME} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME}
PUBLIC
    ./include
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        wot-device-common
        wot-virtual-clock
        WoTCore
        cpp-logger
        wot-binding-rpi
)