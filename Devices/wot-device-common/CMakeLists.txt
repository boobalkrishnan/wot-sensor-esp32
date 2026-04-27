project(wot-device-common)

set(SOURCE_FILES
        src/DeviceImp.cpp
        src/NwAdapter.cpp
        )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -funsigned-char -fpermissive")

add_library(${PROJECT_NAME} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME}
        PUBLIC
        ./include
)

target_link_libraries(${PROJECT_NAME}
        PUBLIC
                WoTApp
                wot-virtual-clock
                WoTCore
                cpp-logger
                wot-binding-rpi
                libcoap::coap-3
        )