#include <iostream>
#include <regex>
#include <string>
#include <vector>
// #include <cJSON.h>
#include "BindClient.h"
#include "Logger.h"


using namespace Poco::Net;
using namespace Poco;

// Perform HTTP GET and return body
std::string httpGet(const std::string &host, unsigned short port, const std::string &uri)
{
    HTTPClientSession session(host, port);
    HTTPRequest request(HTTPRequest::HTTP_GET, uri, HTTPMessage::HTTP_1_1);
    session.sendRequest(request);

    HTTPResponse response;
    std::istream &rs = session.receiveResponse(response);
    std::cout << "Status: " << response.getStatus() << "\n";
    for (const auto &header : response)
    {
        std::cout << header.first << ": " << header.second << "\n";
    }
    if (response.getStatus() != HTTPResponse::HTTP_OK)
    {
        throw std::runtime_error("GET failed: " +
                                 std::to_string(response.getStatus()));
    }
    std::ostringstream body;
    StreamCopier::copyStream(rs, body);
    std::string responseStr = body.str();
    return (responseStr);
}

std::string BindClient::ReadTD(std::string href_td)
{
    pLogger->PrintLog(LOG_LEVEL_DEBUG, LOGGER_COMP_BINDCLIENT, "SetHandlers call");
    if (ClientType == BIND_HTTP_CLIENT)
    {
        std::string resp = httpGet(HostIp, HostPort, href_td);
        return resp;
    }
    else if (ClientType == BIND_COAP_CLIENT)
    {
    }
}

std::string BindClient::ReadProperty(uint8_t Protocol, std::string ThingID, std::string PropID, std::string href_td)
{
    Poco::URI uri(href_td);
    Poco::JSON::Object eventObj;
    if (ClientType == BIND_WS_CLIENT)
    {
        eventObj.set("thingID", ThingID);
        eventObj.set("messageID", 0);//UUIDGenerator::generateRandom().toString());
        eventObj.set("messageType", "request");
        eventObj.set("operation", "readproperty");
        eventObj.set("name", PropID);
        std::stringstream ss;
        eventObj.stringify(ss);
        std::string resp = "To be implemented";
        return resp;
    }
    else if (ClientType == BIND_HTTP_CLIENT)
    {
        // pLogger->PrintLog(LOG_LEVEL_DEBUG, LOGGER_COMP_BINDCLIENT,"SetHandlers call");
        Poco::URI uri(href_td);
        pLogger->PrintLog(LOG_LEVEL_DEBUG, LOGGER_COMP_BINDCLIENT, uri.getPath().c_str());
        std::string resp = httpGet(uri.getHost(), uri.getPort(), uri.getPath().c_str());
        return resp;
    }
    else if (ClientType == BIND_COAP_CLIENT)
    {
        // CoAP client code to read property
    }

}

void BindClient::SubscribeEvent(uint8_t Protocol, std::string ThingID, std::string EventID, std::string eventHref)
{
    // pLogger->PrintLog(LOG_LEVEL_DEBUG, LOGGER_COMP_BINDCLIENT,"SetHandlers call");
    Poco::URI uri(eventHref);
    Poco::JSON::Object eventObj;
    pLogger->PrintLog(LOG_LEVEL_DEBUG, LOGGER_COMP_BINDCLIENT, uri.getPath().c_str());
    if (ClientType == BIND_WS_CLIENT)
    {
        eventObj.set("thingID", ThingID);
        eventObj.set("messageID", 0);//UUIDGenerator::generateRandom().toString());
        eventObj.set("messageType", "request");
        eventObj.set("operation", "subscribeevent");
        eventObj.set("name", EventID);
        std::stringstream ss;
        eventObj.stringify(ss);
        std::string resp = "To be implemented";
    }
    else if (ClientType == BIND_HTTP_CLIENT)
    {
        std::string resp = httpGet(uri.getHost(), uri.getPort(), uri.getPath().c_str());
        // TODO: Should call Event callback to send the response.
        // No inline response given for events
    }
    else if (ClientType == BIND_COAP_CLIENT)
    {
        // CoAP client code to subscribe to event
    }
}

void BindClient::Start(void)
{
    pLogger->PrintLog(LOG_LEVEL_DEBUG, LOGGER_COMP_BINDCLIENT, ClientType);
    if (ClientType == BIND_HTTP_CLIENT)
    {
    }
    else if (ClientType == BIND_COAP_CLIENT)
    {
    }
    else if (ClientType == BIND_WS_CLIENT)
    {
    }
}

void BindClient::Stop(void)
{
}

BindClient::BindClient(uint8_t ClientNo)
{
    ClientType = ClientNo;
    pLogger = Logger::getInstance();
    pLogger->PrintLog(LOG_LEVEL_DEBUG, LOGGER_COMP_BINDCLIENT, "Constructor call");
}

void BindClient::Initialize(std::string host, unsigned int port)
{
    HostIp = host;
    HostPort = port;
}

void BindClient::SetEventHandler(EventHandler_ptr eventHandler)
{

}

void BindClient::Process(void)
{
}