#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include "esp_log.h"
#include "Logger.h"
#include "BindServer.h"
#if defined(BIND_HTTP_SERVER)
    #include "BindHttpServer.h"
#elif defined(BIND_COAP_SERVER)
    #include "BindCoAPServer.h"
#elif defined(BIND_WS_SERVER)
    #include "BindWebSocket.h"
#else
    #include "BindHttpServer.h"
    #include "BindCoAPServer.h"
    #include "BindWebSocket.h"
#endif

static const char *TAG = "BindServer";
#define LOGGER_COMP_BINDSERVER "BindServer"

void BindServer::EmitEvent(std::string EventID, cJSON *out)
{
    ESP_LOGI(TAG, "EventID:%s",EventID.c_str());
#if defined(BIND_HTTP_SERVER)
    if (HTTP_SERVER == Server_Type)
    {
        BindHttpServer::EmitEvent1(EventID,out);
    }
#endif
#if defined(BIND_COAP_SERVER)
    if ( Server_Type == COAP_SERVER)
    {
        ESP_LOGI(TAG, "Server_Type:%d",COAP_SERVER);
        BindCoAPServer::EmitEvent(EventID,out);
    }
#endif
#if defined(BIND_WS_SERVER)
    if ( Server_Type == WS_SERVER)
    {
        ESP_LOGI(TAG, "Server_Type:%d",COAP_SERVER);
        BindWebSocketServer::EmitEvent(EventID,out);
    }
#endif
}

void BindServer::SetHandlers(Adapter_interaction_get GetHandleIn,
                                 Adapter_interaction_put PutHandleIn, 
                                 Adapter_thing_get thingGetIn,
                                 Adapter_affordance_get affordIn,
                                 Adapter_interaction_get SubsHandleIn)
{
    ESP_LOGI(TAG, "SetHandlers call");

#if defined(BIND_HTTP_SERVER)
    if ( Server_Type == HTTP_SERVER)
    {
        BindHttpServer::SetHandlers(GetHandleIn,PutHandleIn,thingGetIn,affordIn,SubsHandleIn);
    }
#endif
#if defined(BIND_COAP_SERVER)
    if ( Server_Type == COAP_SERVER)
    {
        BindCoAPServer::SetHandlers(GetHandleIn,PutHandleIn,thingGetIn,affordIn,SubsHandleIn);
    }
#endif
#if defined(BIND_WS_SERVER)
    if ( Server_Type == WS_SERVER)
    {
        BindWebSocketServer::SetHandlers(GetHandleIn,PutHandleIn,thingGetIn,affordIn,SubsHandleIn);
    }
#endif
}

void BindServer::Start(void)
{
#if defined(BIND_HTTP_SERVER)
    if ( Server_Type == HTTP_SERVER)
    {
         BindHttpServer::Start();
    }
#endif
#if defined(BIND_COAP_SERVER)
    if ( Server_Type == COAP_SERVER)
    {
        BindCoAPServer::Start();
    }
#endif
#if defined(BIND_WS_SERVER)
    if ( Server_Type == WS_SERVER)
    {
        BindWebSocketServer::Start();
    }
#endif
}

void BindServer::Stop(void)
{
#if defined(BIND_HTTP_SERVER)
    if ( Server_Type == HTTP_SERVER)
    {
        BindHttpServer::Stop();
    }
#endif
#if defined(BIND_COAP_SERVER)
    if ( Server_Type == COAP_SERVER)
    {
        BindCoAPServer::Stop();
    }
#endif
#if defined(BIND_WS_SERVER)
    if ( Server_Type == WS_SERVER)
    {
        BindWebSocketServer::Stop();
    }
#endif
}

#if defined(BIND_HTTP_SERVER)
BindServer::BindServer(void)
#elif defined(BIND_COAP_SERVER)
BindServer::BindServer(void)
#elif defined(BIND_WS_SERVER)
BindServer::BindServer(void)
#else
BindServer::BindServer(void)
#endif
{
    Server_Type=0;
}

void BindServer::Initialize(uint8_t ServerNo,unsigned int port)
{
    pLogger = Logger::getInstance();
    ESP_LOGI(TAG, "Initialize call");

    Server_Type = ServerNo;
    ESP_LOGI(TAG, "Server_Type:%d",Server_Type);

#if defined(BIND_HTTP_SERVER)
    if ( Server_Type == HTTP_SERVER)
    {
        BindHttpServer::Initialize(port);
    }
#endif
#if defined(BIND_COAP_SERVER)
    if ( Server_Type == COAP_SERVER)
    {
        BindCoAPServer::Initialize(port);
    }
#endif
#if defined(BIND_WS_SERVER)
    if ( Server_Type == WS_SERVER)
    {
        BindWebSocketServer::Initialize(port);
    }
#endif
}

void BindServer::Process(void)
{
#if defined(BIND_HTTP_SERVER)
    if ( Server_Type == HTTP_SERVER)
    {
        BindHttpServer::Process();
    }
#endif
#if defined(BIND_COAP_SERVER)
    if ( Server_Type == COAP_SERVER)
    {
        BindCoAPServer::Process();
    }
#endif
#if defined(BIND_WS_SERVER)
    if ( Server_Type == WS_SERVER)
    {
        BindWebSocketServer::Process();
    }
#endif
}