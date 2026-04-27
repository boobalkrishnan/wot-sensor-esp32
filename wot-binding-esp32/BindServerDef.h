#ifndef BINDSERVERDEF
#define BINDSERVERDEF

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <regex>
#include "cJSON.h"


using namespace std;

typedef void (*Adapter_interaction_get)(std::string ThingId, std::string Root_Name, std::string Interaction_Type, std::string Interaction_Name, cJSON *JsonData);// ostream& DataOut);
typedef void (*Adapter_interaction_put)(std::string ThingId, std::string Root_Name, std::string Interaction_Type, std::string Interaction_Name, istream& DataIn, uint32_t Datalen);
typedef void (*Adapter_thing_get)(std::string HostIn, std::string ThingId, std::string Root_Name, cJSON *JsonMeta);
typedef void (*Adapter_affordance_get)(std::string HostIn, std::string ThingId, std::string Root_Name, std::string Interaction_Type, cJSON *ThingDesc);

//#define BIND_WS_SERVER
// #define BIND_HTTP_SERVER
//#define BIND_COAP_SERVER
//#define BIND_SELECTED   BIND_HTTP_SERVER
#endif /* BINDSERVERDEF */