
#ifndef BINDSERVER
#define BINDSERVER

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <regex>
#include "cJSON.h"
#include "BindServer.h"
#include "BindServerDef.h"
#include "Logger.h"
using namespace std;

#define HTTP_SERVER  (1)
#define COAP_SERVER  (2)
#define WS_SERVER    (3)

#ifdef BIND_HTTP_SERVER
        #include "BindHttpServer.h"
        class BindServer: public BindHttpServer {
#else
        #ifdef BIND_COAP_SERVER
                #include "BindCoAPServer.h"
                class BindServer: public BindCoAPServer {
        #else 
                #ifdef BIND_WS_SERVER
                        #include "BindWebSocket.h"
                        class BindServer: public BindWebSocketServer {
                #else
                        #include "BindHttpServer.h"
                        #include "BindCoAPServer.h"
                        #include "BindWebSocket.h"
                        class BindServer: public BindHttpServer, BindCoAPServer, BindWebSocketServer {
                #endif
        #endif
#endif
        public:
        BindServer(void);
        uint8_t Server_Type;
// #if defined(BIND_HTTP_SERVER)
//         BindHttpServer MyServer;
// #elif defined(BIND_COAP_SERVER)
//         BindCoAPServer MyServer;
// #elif defined(BIND_WS_SERVER)
//         BindWebSocketServer MyServer;
// #else
//         BindHttpServer  *HttpServer;
//         BindCoAPServer  *CoAPServer;
// #endif
        Logger* pLogger = NULL; // Create the object pointer for Logger Class

        void EmitEvent(std::string EventID, cJSON *out);
        void SetHandlers(Adapter_interaction_get GetHandleIn,
                         Adapter_interaction_put SetHandleIn,
                         Adapter_thing_get thingGetIn,
                         Adapter_affordance_get affordIn,
                         Adapter_interaction_get SubsHandleIn);

        void Start(void);
        void Stop(void);
        void Initialize(uint8_t ServerNo,unsigned int port);
        void Process(void);
};

#endif