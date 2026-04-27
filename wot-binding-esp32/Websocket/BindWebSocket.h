
#include "Logger.h"
#include "esp_event.h"
#include "esp_http_server.h"
#include "BindServerDef.h"

#ifndef BINDWEBSOCKET
#define BINDWEBSOCKET

using namespace std;
using namespace Logging;

#define LOGGER_COMP_HTTPSERVER "WebSocketServer"

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

extern Adapter_interaction_get HandleSubscription;
extern Adapter_interaction_get HandleAffordanceGet;
extern Adapter_interaction_put HandleAffordancePut;
extern Adapter_thing_get HandleThing;
extern Adapter_affordance_get HandleAffordance;

extern void EmitSubscribedEvent(std::string EventID, std::string out);

class BindWebSocketServer //: public ServerApplication
{
    private:
 
    public:
        // HTTPServer *WifiHotspot;
        // httpd_handle_t  WotServer = NULL;
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
        BindWebSocketServer(void);
        void Initialize(unsigned int port);
        void Process(void);
        // static esp_err_t any_handler(httpd_req_t *req);
        void Start(void);
        void Stop(void);
};

#endif /* BINDWEBSOCKET */
