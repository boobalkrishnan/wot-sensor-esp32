project(cpp-logger)

set(SOURCE_FILES
        src/Logger.cpp)

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
        wot-binding-rpi
)