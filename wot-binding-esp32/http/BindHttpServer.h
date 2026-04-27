
#include "Logger.h"
#include "BindServer.h"
#include "esp_event.h"
#include "esp_http_server.h"

#ifndef BINDHTTPSERVER
#define BINDHTTPSERVER

using namespace std;
using namespace Logging;

#define LOGGER_COMP_HTTPSERVER "HttpServer"

enum UriLevel{
  URI_THINGS_LEVEL=1,
  URI_THING_LEVEL,
  URI_INTERACT_LEVEL,
};

// #define URI_THINGS_INDEX        0
#define URI_THING_INDEX         0
#define URI_AFFORDANCES_INDEX   1
#define URI_AFFORDANCE_INDEX    2
#define URI_SUBSCRIPTION_INDEX  3

#define PROPERTY_INTERACT "property"
#define ACTION_INTERACT "action"
#define EVENT_INTERACT "event"

// typedef bool (*Adapter_callback_get)(std::string RxURI, Poco::JSON::Object *ThingDesc);
// typedef bool (*Adapter_callback_put)(std::string RxURI, Poco::JSON::Object::Ptr JsonMeta);
// typedef void (*Adapter_interaction_get)(std::string ThingId, std::string Root_Name, std::string Interaction_Type, std::string Interaction_Name, ostream& DataOut);
// typedef void (*Adapter_interaction_put)(std::string ThingId, std::string Root_Name, std::string Interaction_Type, std::string Interaction_Name, istream& DataIn, uint32_t Datalen);
// typedef void (*Adapter_thing_get)(std::string HostIn, std::string ThingId, std::string Root_Name, Poco::JSON::Object *JsonMeta);
// typedef void (*Adapter_affordance_get)(std::string HostIn, std::string ThingId, std::string Root_Name, std::string Interaction_Type, Poco::JSON::Object *ThingDesc);

// class BindHttpServerHandler : public HTTPRequestHandler
// {
//     private:
//         void PropertyReadWriteHandler();
//     public:
//         Logger* pLogger = NULL; // Create the object pointer for Logger Class
//         unsigned int ParseThingsURI(std::string RxURI,std::string* ArrayOfURI);
//         virtual void handleRequest(HTTPServerRequest &req, HTTPServerResponse &resp);
// };


extern Adapter_interaction_get HandleSubscription;
extern Adapter_interaction_get HandleAffordanceGet;
extern Adapter_interaction_put HandleAffordancePut;
extern Adapter_thing_get HandleThing;
extern Adapter_affordance_get HandleAffordance;

extern void EmitSubscribedEvent(std::string EventID, std::string out);

class BindHttpServer //: public ServerApplication
{
    private:
 
        // HTTPServer *WifiHotspot;
        httpd_handle_t  WotServer = NULL;
    public:
        /**
        * Construct a new Triangle object from another Triangle object.
        * @brief Copy constructor.
        * @param triangle Another Triangle object.

        */
        unsigned int Port;
        std::string Address;
        int main(const vector<string> &);
        Logger* pLogger = NULL; // Create the object pointer for Logger Class
        unsigned int count;
        unsigned int ParseThingsURI(std::string RxURI,std::string* ArrayOfURI);
        // static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
        void wifi_init_sta(void);
        void EmitEvent(std::string EventID, cJSON *out);
        void SetHandlers(Adapter_interaction_get GetHandleIn,
                         Adapter_interaction_put SetHandleIn,
                         Adapter_thing_get thingGetIn,
                         Adapter_affordance_get affordIn,
                         Adapter_interaction_get SubsHandleIn);
        BindHttpServer(void);
        void Initialize(unsigned int port);
        void Process(void);
        // static esp_err_t any_handler(httpd_req_t *req);
        void Start(void);
        void Stop(void);
};

#endif /* BINDHTTPSERVER */
