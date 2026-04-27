#ifndef BINDCLIENT
#define BINDCLIENT

#include "Logger.h"
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using namespace Logging;

#define LOGGER_COMP_BINDCLIENT "BindClient"
#define BIND_HTTP_CLIENT 1
#define BIND_COAP_CLIENT 2
#define BIND_WS_CLIENT 3
#define BIND_MQTT_CLIENT 4

typedef void (*EventHandler_ptr)(std::string ThingID, std::string EventID, Poco::JSON::Object::Ptr eventData);

class BindClient
{
public:
    uint8_t ClientType;
    std::string HostIp;
    unsigned int HostPort;
    Logger *pLogger = NULL; // Create the object pointer for Logger Class
    unsigned int count;
    void EmitEvent(std::string EventID, std::string out);
    std::string ReadTD(std::string href_td);
    BindClient(uint8_t ClientNo);
    void Start(void);
    void Stop(void);
    void Initialize(std::string host, unsigned int port);
    void Process(void);
    std::string ReadProperty(uint8_t Protocol, std::string ThingID, std::string PropID, std::string propertyHref);
    void SubscribeEvent(uint8_t Protocol, std::string ThingID, std::string EventID, std::string eventHref);
    void SetEventHandler(EventHandler_ptr eventHandler);
};

#endif /* BINDCLIENT */
