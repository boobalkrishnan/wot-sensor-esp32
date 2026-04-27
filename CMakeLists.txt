cmake_minimum_required(VERSION 3.13)
project(wot-gateway)

include(FetchContent)
set(POCO_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Library/poco)

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/Library/poco/install" CACHE PATH "Install path" FORCE)
find_package(Poco CONFIG QUIET COMPONENTS Foundation Net Util XML JSON DNSSD DNSSDAvahi)
# If not found, fetch from Git
if (NOT Poco_FOUND)
    message(STATUS "Poco not found, fetching from GitHub...")

    FetchContent_Declare(
        poco
        GIT_REPOSITORY https://github.com/pocoproject/poco.git
        GIT_TAG        poco-1.14.2-release  # or a specific release tag like 'poco-1.12.4-release'
        SOURCE_DIR     ${POCO_SOURCE_DIR}
    )
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
    set(ENABLE_DNSSD ON CACHE BOOL "Enable DNSSD support" FORCE)
    set(ENABLE_DNSSD ON CACHE BOOL "Enable DNSSD support" FORCE)
    set(ENABLE_DNSSD_AVAHI ON CACHE BOOL "Enable DNSSD Avahi support" FORCE)
    set(ENABLE_NETSSL OFF CACHE BOOL "Disable NetSSL" FORCE)
    set(ENABLE_CRYPTO OFF CACHE BOOL "Disable Crypto" FORCE)
    set(ENABLE_DATA OFF CACHE BOOL "Disable Data" FORCE)
    set(ENABLE_DATA_SQLITE OFF CACHE BOOL "Disable SQLite" FORCE)
    set(ENABLE_MONGODB OFF CACHE BOOL "Disable MongoDB" FORCE)
    set(ENABLE_REDIS OFF CACHE BOOL "Disable Redis" FORCE)
    set(ENABLE_PROMETHEUS OFF CACHE BOOL "Disable Prometheus" FORCE)

    FetchContent_MakeAvailable(poco)
    
    # Set Poco targets for linking
    set(POCO_LIBRARIES Poco::Foundation Poco::Net Poco::Util Poco::JSON DNSSD DNSSDAvahi)
else()
    # Use found Poco libraries
    set(POCO_LIBRARIES ${Poco_LIBRARIES})
endif()

# find_package(Poco CONFIG QUIET COMPONENTS DNSSD DNSSDAvahi)
# if (NOT Poco_DNSSD_FOUND)
#     message(STATUS "Poco DNSSD module not found, fetching separately...")
#     # Fetch DNSSD module separately
#     FetchContent_Declare(
#         poco_dnssd
#         GIT_REPOSITORY https://github.com/pocoproject/poco-dnssd.git
#         GIT_TAG        master
#         SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/Library/poco-dnssd
#     )

#     FetchContent_MakeAvailable(poco_dnssd)
#     list(APPEND POCO_LIBRARIES DNSSD DNSSDAvahi)
# endif()

# Try to find libcoap 3
# set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Library/libcoap/install")
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/Library/libcoap/install")
find_package(libcoap-3 QUIET)
if (NOT libcoap-3_FOUND)
    message(STATUS "libcoap-3 not found, fetching from GitHub...")

    set(LIBCOAP_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Library/libcoap)

    FetchContent_Declare(
        libcoap
        GIT_REPOSITORY https://github.com/obgm/libcoap.git
        GIT_TAG        v4.3.5  # or use a stable tag like 'v4.3.0'
        SOURCE_DIR     ${LIBCOAP_SOURCE_DIR}
    )

    # Optional: configure libcoap build
    set(ENABLE_DTLS OFF CACHE BOOL "" FORCE)
    set(ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(ENABLE_TESTS OFF CACHE BOOL "" FORCE)
    set(ENABLE_DOCS OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(libcoap)
endif()

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/Library/json-schema-validator/install")
find_package(nlohmann_json_schema_validator QUIET)
if (NOT nlohmann_json_schema_validator_FOUND)
    message(STATUS "nlohmann_json_schema_validator not found, fetching from GitHub...")
    set(JSONVALID_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Library/json-schema-validator)

    FetchContent_Declare(
            json_validator
            GIT_REPOSITORY https://github.com/pboettch/json-schema-validator.git
            GIT_TAG "2.3.0"
            SOURCE_DIR     ${JSONVALID_SOURCE_DIR}
        )

    FetchContent_MakeAvailable(json_validator)
endif()
# target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)
# set(CMAKE_PREFIX_PATH ./../../Library/libserial/build/lib)

#################### CJSON ##################
# TODO: Could not build with package FetchContent_MakeAvailable. The CJSON package has built before include here
# set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Library/cJSON/install/lib/cmake/cJSON/")
# find_package(cJSON REQUIRED)

# include_directories(${CJSON_INCLUDE_DIRS})
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/Library/cJSON/install")
find_package(cjson QUIET)

if (NOT cjson_FOUND)
    message(STATUS "cJSON not found, fetching from GitHub...")

    set(CJSON_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Library/cJSON)

    FetchContent_Declare(
        cJSON
        GIT_REPOSITORY https://github.com/DaveGamble/cJSON.git
        GIT_TAG        v1.7.18
        SOURCE_DIR     ${CJSON_SOURCE_DIR}
    )
    set(ENABLE_CUSTOM_COMPILER_FLAGS  OFF CACHE BOOL "" FORCE)
    set(ENABLE_CJSON_TEST OFF CACHE BOOL "" FORCE)
    set(ENABLE_CJSON_UTILS OFF CACHE BOOL "" FORCE)
    set(ENABLE_PUBLIC_SYMBOLS OFF CACHE BOOL "" FORCE)
    set(ENABLE_CJSON_UNINSTALL OFF CACHE BOOL "" FORCE)
    set(ENABLE_LOCALES OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(cJSON)
    # find_package(PkgConfig)
    # pkg_check_modules(CJSON cjson)
    # include_directories(${CJSON_INCLUDE_DIRS})
endif()

find_package(PkgConfig)
pkg_check_modules(SERIAL libserial)
include_directories(${SERIAL_INCLUDE_DIRS})

add_subdirectory(Utility)
add_subdirectory(App)
add_subdirectory(Devices)

add_subdirectory(wot-binding-rpi)
add_subdirectory(wotcpp)

SET(SOURCE_FILES
    ./main.cpp
    )

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME}
	PUBLIC
        # Poco libraries
        ${POCO_LIBRARIES}
        # Ethernet
		nlohmann_json_schema_validator
        WoTApp
        wot-device-common
        wot-virtual-clock
        WoTCore
        cpp-logger
        wot-binding-rpi
        libcoap::coap-3
        ${SERIAL_LINK_LIBRARIES}
        cjson
	)

#target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -static-libasan)
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

### Build documentation ###
# if(DOCUMENTATION)
#     add_subdirectory(Doc)
# endif()