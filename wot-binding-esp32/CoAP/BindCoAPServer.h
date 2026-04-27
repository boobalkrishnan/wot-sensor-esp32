#ifndef BINDCOAPSERVER
#define BINDCOAPSERVER
#include "BindServerDef.h"
#include "Logger.h"

using namespace std;
using namespace Logging;

enum CoAP_UriLevel{
  CoAP_URI_THINGS_LEVEL=1,
  CoAP_URI_THING_LEVEL,
  CoAP_URI_INTERACT_LEVEL,
};

#define CoAP_URI_THING_INDEX         0
#define CoAP_URI_AFFORDANCES_INDEX   1
#define CoAP_URI_AFFORDANCE_INDEX    2
#define CoAP_URI_SUBSCRIPTION_INDEX  3

#define PROPERTY_INTERACT "property"
#define ACTION_INTERACT "action"
#define EVENT_INTERACT "event"

#define LOGGER_COMP_COAPSERVER  "CoapServer"

// typedef void (*CoapAdapter_interaction_get)(std::string ThingId, std::string Root_Name, std::string Interaction_Type, std::string Interaction_Name, ostream& DataOut);
// typedef void (*CoapAdapter_interaction_put)(std::string ThingId, std::string Root_Name, std::string Interaction_Type, std::string Interaction_Name, istream& DataIn, uint32_t Datalen);
// typedef void (*CoapAdapter_thing_get)(std::string HostIn, std::string ThingId, std::string Root_Name, Poco::JSON::Object *JsonMeta);
// typedef void (*CoapAdapter_affordance_get)(std::string HostIn, std::string ThingId, std::string Root_Name, std::string Interaction_Type, Poco::JSON::Object *ThingDesc);

class BindCoAPServer //: public ServerApplication
{
    private:

    public:
        /**
        * Construct a new Triangle object from another Triangle object.
        * @brief Copy constructor.
        * @param triangle Another Triangle object.

        */
        unsigned int Port;
        std::string Address;
        void Initialize(unsigned int port);
        void Process(void);
        Logger* pLogger = NULL; // Create the object pointer for Logger Class
        unsigned int count;
        void EmitEvent(std::string EventID,  cJSON *out);
        void SetHandlers(Adapter_interaction_get GetHandleIn,
                              Adapter_interaction_put PutHandleIn, 
                              Adapter_thing_get thingGetIn,
                              Adapter_affordance_get affordIn,
                              Adapter_interaction_get SubsHandleIn);
        BindCoAPServer(void);
        void Start(void);
        void Stop(void);
};
#endif /* BINDCOAPSERVER */
