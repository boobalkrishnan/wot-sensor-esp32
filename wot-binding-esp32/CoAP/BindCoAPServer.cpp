
#include <iostream>
#include <streambuf>
#include <string>

#include <sys/socket.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"

#include <errno.h>
#include "BindServerDef.h"
#include "BindCoAPServer.h"
#include "Logger.h"

#include "coap3/coap.h"

/* The examples use simple WiFi configuration that you can set via
   'make menuconfig'.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_SSID      "dlink-boobal" //CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS      "boobal-55" //CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_MAXIMUM_RETRY  CONFIG_ESP_MAXIMUM_RETRY

#if CONFIG_ESP_WPA3_SAE_PWE_HUNT_AND_PECK
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HUNT_AND_PECK
#define EXAMPLE_H2E_IDENTIFIER ""
#elif CONFIG_ESP_WPA3_SAE_PWE_HASH_TO_ELEMENT
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HASH_TO_ELEMENT
#define EXAMPLE_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID
#elif CONFIG_ESP_WPA3_SAE_PWE_BOTH
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_BOTH
#define EXAMPLE_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID
#endif
#if CONFIG_ESP_WIFI_AUTH_OPEN
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
#elif CONFIG_ESP_WIFI_AUTH_WEP
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
#elif CONFIG_ESP_WIFI_AUTH_WPA_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WAPI_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK
#endif

#define HTTP_ANY INT_MAX

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

#define COAP_DEFAULT_TIME_SEC 5
#define COAP_DEFAULT_TIME_USEC 0

static EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const static int CONNECTED_BIT = BIT0;

const static char *TAG = "CoAP_server";
static int s_retry_num = 0;
using namespace Logging;

// #define URI_THINGS_INDEX        0
#define URI_THING_INDEX         0
#define URI_AFFORDANCES_INDEX   1
#define URI_AFFORDANCE_INDEX    2
#define URI_SUBSCRIPTION_INDEX  3
#define MAX_EVENTS 10
Logger* CoAP_pLogger = NULL; // Create the object pointer for Logger Class

Adapter_interaction_get CoapHandleSubscription;
Adapter_interaction_get CoapHandleAffordanceGet;
Adapter_interaction_put CoapHandleAffordancePut;
Adapter_thing_get CoapHandleThing;
Adapter_affordance_get CoapHandleAffordance;

std::string decodeURIComponent(std::string encoded) {

    std::string decoded = encoded;
    std::smatch sm;
    std::string haystack;

    int dynamicLength = decoded.size() - 2;

    if (decoded.size() < 3) return decoded;

    for (int i = 0; i < dynamicLength; i++)
    {

        haystack = decoded.substr(i, 3);

        if (std::regex_match(haystack, sm, std::regex("%[0-9A-F]{2}")))
        {
            haystack = haystack.replace(0, 1, "0x");
            std::string rc = {(char)std::stoi(haystack, nullptr, 16)};
            decoded = decoded.replace(decoded.begin() + i, decoded.begin() + i + 3, rc);
        }

        dynamicLength = decoded.size() - 2;

    }

    return decoded;
}

unsigned int CoAP_ParseThingsURI(std::string RxURI,std::string* ArrayOfURI)
{
    int LevelOfUTI=0;
    std::string delimiter = "/";

    size_t pos = 0;
    std::string token;
	int position = RxURI.find(":");
	std::string RxURI_Local = decodeURIComponent(RxURI);
    cout << "Parsing:" << RxURI_Local << std::endl;
	// RxURI_Local.replace(0,"%2F","/");
	cout << "Delimiter is:" << delimiter << std::endl;
    while ((pos = RxURI_Local.find(delimiter)) != std::string::npos)
    {
        // cout << "Parsing:" << RxURI << std::endl;  
        token = RxURI_Local.substr(0, pos);
        if (token != "")
        {
            ArrayOfURI[LevelOfUTI]=token;
            LevelOfUTI++;
        }
        RxURI_Local.erase(0, pos + delimiter.length());

    }
    cout << "Parsed:" << RxURI_Local << std::endl;
    ArrayOfURI[LevelOfUTI]=RxURI_Local;
    // pLogger->PrintLog(LOG_LEVEL_DEBUG, LOGGER_COMP_HTTPSERVER,"Parsed paths:",LevelOfUTI);
    // cout << "Parsed Levels:" << LevelOfUTI << std::endl; 
    return (LevelOfUTI);  
}

struct membuf : std::streambuf
{
    membuf(char* begin, char* end) {
        this->setg(begin, begin, end);
    }
};

std::string ThingURI[10];

void CoAP_GetHandler(coap_resource_t *resource, coap_session_t *session, const coap_pdu_t *request, const coap_string_t *query, coap_pdu_t* pResponse)
{
    cJSON *Response = cJSON_CreateObject();
    std::string Host;
    std::string RequestMethod = "GET";	

    std::string Interaction_Type="";
    std::string Interaction_Name="";
    std::string Thing_Name;
    std::string Root_Name;
    unsigned int RequestLevel;
	static coap_resource_t *resource_local;
	resource_local = resource;
	coap_string_t *uri= coap_get_uri_path(request); //->get_request_uri();
	// coap_string_t *uri= coap_get_query(request); //->get_request_uri();
	ESP_LOGI(TAG, "Request URI=%s",(const char*)uri->s);
	// CoAP_pLogger->PrintLog(LOG_LEVEL_DEBUG, LOGGER_COMP_COAPSERVER,(const char*)uri->s);//, uri);
	std::string Request_URI = (const char*)uri->s;

	cout << "Request URI= %s" << Request_URI<< endl;
	RequestLevel = CoAP_ParseThingsURI(Request_URI,ThingURI);
	ESP_LOGI(TAG, "Requested Path Level:%d", RequestLevel);
	// CoAP_pLogger->PrintLog(LOG_LEVEL_DEBUG, LOGGER_COMP_COAPSERVER,"Requested Path Level:",RequestLevel);

    switch(RequestLevel)
    {
        // case URI_THINGS_INDEX:
        case CoAP_URI_THING_INDEX:
        {
            Root_Name = ThingURI[0];
            Thing_Name = ThingURI[0];
            if (RequestMethod == "GET")
            {
                CoapHandleThing(Host,Thing_Name,Root_Name, Response);
                // Response.stringify(out);
                // Response.stringify(ResponseStr);
                // pLogger->PrintLog(LOG_LEVEL_DEBUG, LOGGER_COMP_HTTPSERVER,ResponseStr.str().c_str());                
            }
        }
        break;
        /* Handler for specific Interaction */
        case CoAP_URI_AFFORDANCE_INDEX:
        {
            Interaction_Name = ThingURI[CoAP_URI_AFFORDANCE_INDEX]; // Name of the interaction
        }
        /* Handler for reteriving all Interaction */
        case CoAP_URI_AFFORDANCES_INDEX:
        {
            Root_Name = ThingURI[0];    // Root Name is not used
            Thing_Name = ThingURI[0];
            if (ThingURI[CoAP_URI_AFFORDANCES_INDEX] == PROPERTY_INTERACT)
            {
                Interaction_Type = PROPERTY_INTERACT;
            }
            else if (ThingURI[URI_AFFORDANCES_INDEX] == ACTION_INTERACT)
            {
                Interaction_Type = ACTION_INTERACT;
            }
            else if (ThingURI[URI_AFFORDANCES_INDEX] == EVENT_INTERACT)
            {
                Interaction_Type = EVENT_INTERACT;
            }
            ESP_LOGI(TAG,"Interaction Type: %s",Interaction_Type.c_str());
            ESP_LOGI(TAG,"Interaction Name: %s",Interaction_Name.c_str());
            if (Interaction_Name == "")
            {
                /* Handler for reteriving all Interaction */
                CoapHandleAffordance(Host, Thing_Name, Root_Name, Interaction_Type, Response);
                // Response.stringify(out);
            }
            else
            {
                /* Handler for specific Interaction request*/
                if (RequestMethod == "GET")
                {
                    /* Data will be updated in ostream out parameter in handler function*/
                    CoapHandleAffordanceGet(Thing_Name, Root_Name, Interaction_Type,Interaction_Name, Response);
                    // Response.stringify(out);
                }
                else
                {

                }
            }
        }
        break;
    }

	coap_pdu_set_code(pResponse, (coap_pdu_code_t)COAP_RESPONSE_CODE(203));
	const char* resp_str = cJSON_PrintUnformatted (Response);
	std::string data=resp_str;
	ESP_LOGI(TAG,"Data to be sent:%s",data.c_str());
	// coap_add_data(pResponse, data.size(), (const uint8_t*)resp_str);
    // if (coap_send(ctx, local_if, &async->peer, response) == COAP_INVALID_TID) {

    // }
    coap_add_data_large_response(resource, session, request, pResponse,
                                 query, COAP_MEDIATYPE_APPLICATION_JSON, 60, 0,
                                 data.size(),
                                 (const uint8_t*)resp_str,
                                 NULL, NULL);
}

static void coap_log_handler (coap_log_t level, const char *message)
{
    uint32_t esp_level = ESP_LOG_INFO;
    const char *cp = strchr(message, '\n');

    while (cp) {
        ESP_LOG_LEVEL(esp_level, TAG, "%.*s", (int)(cp - message), message);
        message = cp + 1;
        cp = strchr(message, '\n');
    }
    if (message[0] != '\000') {
        ESP_LOG_LEVEL(esp_level, TAG, "%s", message);
    }
}

static void coap_demo_thread(void *p)
{
    coap_context_t*  ctx = NULL;
    coap_address_t   serv_addr;
    coap_resource_t* resource = NULL;
    fd_set           readfds;
    struct timeval tv;
    int flags = 0;

    /* Initialize libcoap library */
    coap_startup();
    // coap_set_log_handler(coap_log_handler);
    // coap_set_log_level(EXAMPLE_COAP_LOG_DEFAULT_LEVEL);

	unsigned wait_ms;
	ESP_LOGI(TAG, "coap_demo_thread() start");
	/* Prepare the CoAP server socket */
	coap_address_init(&serv_addr);
	serv_addr.addr.sin.sin_family      = AF_INET;
	serv_addr.addr.sin.sin_addr.s_addr = INADDR_ANY;
	serv_addr.addr.sin.sin_port        = htons(COAP_DEFAULT_PORT);
	ctx = coap_new_context(&serv_addr);

	if (!ctx) {
		ESP_LOGE(TAG, "coap_new_context() failed");

	}
	// coap_context_set_block_mode(ctx,
	// 							COAP_BLOCK_USE_LIBCOAP | COAP_BLOCK_SINGLE_BODY);
	// coap_context_set_max_idle_sessions(ctx, 20);
	// coap_context_set_keepalive(ctx, 30);
	ESP_LOGI(TAG, "coap_new_endpoint() start");
	coap_endpoint_t* pCoapEndpoint = coap_new_endpoint(ctx, &serv_addr, COAP_PROTO_UDP);
	if (!pCoapEndpoint) {ESP_LOGW(TAG, "cannot create endpoint for proto");}
 	resource = coap_resource_unknown_init2(CoAP_GetHandler, 0);
	//resource = coap_resource_init(NULL, 0);
	if (!resource) {
		ESP_LOGE(TAG, "coap_resource_init() failed");
	}
	coap_register_request_handler(resource, COAP_REQUEST_GET, CoAP_GetHandler);
	coap_add_resource(ctx, resource);

	wait_ms = COAP_RESOURCE_CHECK_TIME * 1000;
	ESP_LOGI(TAG, "while() start");
	while (1) {
		int result = coap_io_process(ctx, wait_ms);
		if (result < 0) {
			break;
		} else if (result && (unsigned)result < wait_ms) {
			/* decrement if there is a result wait time returned */
			wait_ms -= result;
		}
		if (result) {
			/* result must have been >= wait_ms, so reset wait_ms */
			wait_ms = COAP_RESOURCE_CHECK_TIME * 1000;
		}
		// if (ctx != NULL){
		// 	coap_run_once(ctx, 0);
		// }
	}
	ESP_LOGI(TAG, "while() end");
    coap_free_context(ctx);
    coap_cleanup();
    vTaskDelete(NULL);
}

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < 5) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());    
#if 0

#endif
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (pasword len => 8).
             * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
             * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
             * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
             */
            // .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
            // .sae_pwe_h2e = ESP_WIFI_SAE_MODE,
            // .sae_h2e_identifier = EXAMPLE_H2E_IDENTIFIER,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

}

void BindCoAPServer::EmitEvent(std::string EventID, cJSON *out)
{
    cout<< "EventID:"<<EventID.c_str()<<endl;
}

// static void *CoAP_Process(void*)
void BindCoAPServer::Process(void)
{


}

void BindCoAPServer::SetHandlers(Adapter_interaction_get GetHandleIn,
                                 Adapter_interaction_put PutHandleIn, 
                                 Adapter_thing_get thingGetIn,
                                 Adapter_affordance_get affordIn,
                                 Adapter_interaction_get SubsHandleIn)
{
	CoAP_pLogger->PrintLog(LOG_LEVEL_DEBUG, LOGGER_COMP_COAPSERVER,"SetHandlers call");    
    CoapHandleAffordanceGet = GetHandleIn;
    CoapHandleAffordancePut = PutHandleIn;
    CoapHandleThing = thingGetIn;
    CoapHandleAffordance = affordIn;
    CoapHandleSubscription = SubsHandleIn;
}


void BindCoAPServer::Initialize(unsigned int port)
{
	CoAP_pLogger = Logger::getInstance();
    Port = port;
	wifi_init_sta();
    xTaskCreate(coap_demo_thread, "coap", 4096, NULL, 5, NULL);
}

BindCoAPServer::BindCoAPServer(void)
{

}

void BindCoAPServer::Start(void)
{
	CoAP_pLogger->PrintLog(LOG_LEVEL_DEBUG, LOGGER_COMP_COAPSERVER,"CoAP Server starting");
}

void BindCoAPServer::Stop(void)
{

}