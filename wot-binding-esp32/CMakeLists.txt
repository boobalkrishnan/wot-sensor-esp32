
#set(USED_BINDING_INTERFACE "HTTP") #OFF by default

if (USED_BINDING_INTERFACE MATCHES "HTTP")
    set(SRC_DIRS_LIST       ${SRC_DIRS_LIST}
                            "Bindings"
                            "Bindings/http")

    set(INCLUDE_DIRS_LIST   ${INCLUDE_DIRS_LIST}
                            "Bindings"
                            "Bindings/http")

    set(EXCLUDE_SRCS_LIST )
    #target_compile_definitions(${COMPONENT_TARGET} PUBLIC "-DBIND_HTTP_SERVER")
    #set(CXX_COMPILER_OPTIONS "${CXX_COMPILER_OPTIONS} -D_BIND_HTTP_SERVER")
elseif (USED_BINDING_INTERFACE MATCHES "COAP")
    set(SRC_DIRS_LIST       ${SRC_DIRS_LIST}
                            "Bindings"
                            "Bindings/CoAP")

    set(INCLUDE_DIRS_LIST   ${INCLUDE_DIRS_LIST}
                            "Bindings"
                            "Bindings/CoAP")

    set(EXCLUDE_SRCS_LIST )
    #set(CXX_COMPILER_OPTIONS "${CXX_COMPILER_OPTIONS} -D_BIND_COAP_SERVER")
    #target_compile_definitions(${COMPONENT_TARGET} PUBLIC "-DBIND_COAP_SERVER")
elseif (USED_BINDING_INTERFACE MATCHES "WS")
    set(SRC_DIRS_LIST       ${SRC_DIRS_LIST}
                            "Bindings"
                            "Bindings/Websocket")

    set(INCLUDE_DIRS_LIST   ${INCLUDE_DIRS_LIST}
                            "Bindings"
                            "Bindings/Websocket")

    set(EXCLUDE_SRCS_LIST )
    #set(CXX_COMPILER_OPTIONS "${CXX_COMPILER_OPTIONS} -D_BIND_WS_SERVER")
    #target_compile_definitions(${COMPONENT_TARGET} PUBLIC "-DBIND_WS_SERVER")
elseif (USED_BINDING_INTERFACE MATCHES "ALL")
    set(SRC_DIRS_LIST       ${SRC_DIRS_LIST}
                            "Bindings"
                            "Bindings/http"
                            "Bindings/Websocket"
                            "Bindings/CoAP")

    set(INCLUDE_DIRS_LIST   ${INCLUDE_DIRS_LIST}
                            "Bindings"
                            "Bindings/http"
                            "Bindings/Websocket"
                            "Bindings/CoAP")
    #set(CXX_COMPILER_OPTIONS "${CXX_COMPILER_OPTIONS} -D_BIND_HTTP_SERVER -D_BIND_COAP_SERVER -D_BIND_WS_SERVER")
    #target_compile_definitions(${COMPONENT_TARGET} PUBLIC "-DBIND_HTTP_SERVER -DBIND_COAP_SERVER -DBIND_WS_SERVER")
    set(EXCLUDE_SRCS_LIST )

endif()
