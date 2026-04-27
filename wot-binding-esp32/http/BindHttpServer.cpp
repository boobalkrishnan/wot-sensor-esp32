#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>
#include "esp_tls_crypto.h"
#include "esp_http_server.h"
#include "esp_check.h"
#include <time.h>
#include <sys/time.h>
#include "lwip/err.h"
#include "lwip/sys.h"

#include "BindServer.h"
#include "BindHttpServer.h"
#include "Logger.h"

using namespace std;

/* The examples use WiFi configuration that you can set via project configuration menu

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



/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char *TAG = "wifi station";

static int s_retry_num = 0;


/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;
//pthread_mutex_t Handler_Mutex;
pthread_mutex_t Event_Mutex;
pthread_cond_t Event_cv;
pthread_mutex_t Evnt_Subscribe_Mutex;
pthread_cond_t Evnt_Subscribe_cv;
std::string Evnt_SubscribeData;
//std::string EventData;
// unsigned int  BindHttpServerHandler::count = 0;
Adapter_interaction_get HandleSubscription;
Adapter_interaction_get HandleAffordanceGet;
Adapter_interaction_put HandleAffordancePut;
Adapter_thing_get HandleThing;
Adapter_affordance_get HandleAffordance;
uint8_t counter=0;
typedef struct{
    std::string Evnt_SubscribeHandlers;
    // HTTPServerResponse &sseResp;
    std::string Evnt_SubscribeData;
}Evnt_SubscribeHandlers_t;

uint8_t Evnt_SubscribeRequestsCnt=0;

Evnt_SubscribeHandlers_t Evnt_SubscribeHandlers[100];
unsigned int ParseThingsURI(std::string RxURI,std::string* ArrayOfURI)
{
    std::string delimiter = "/";
    int LevelOfUTI=0;
    size_t pos = 0;
    std::string token;
    cout << "Parsing:" << RxURI << std::endl;
    while ((pos = RxURI.find(delimiter)) != std::string::npos)
    {
        // cout << "Parsing:" << RxURI << std::endl;  
        token = RxURI.substr(0, pos);
        if (token != "")
        {
            ArrayOfURI[LevelOfUTI]=token;
            LevelOfUTI++;
        }
        RxURI.erase(0, pos + delimiter.length());

    }
    cout << "Parsed:" << RxURI << std::endl;
    ArrayOfURI[LevelOfUTI]=RxURI;
    // pLogger->PrintLog(LOG_LEVEL_DEBUG, LOGGER_COMP_HTTPSERVER,"Parsed paths:",LevelOfUTI);
    cout << "Parsed Levels:" << LevelOfUTI << std::endl; 
    return (LevelOfUTI);  
}

void EmitSubscribedEvent(std::string EventID, cJSON *out)
{
    cout<< "EmitSubscribedEvent Start"<<endl;
    pthread_mutex_lock(&Evnt_Subscribe_Mutex);
    for (uint8_t eventcnt=0;eventcnt<Evnt_SubscribeRequestsCnt;eventcnt++)
    {
        cout<< "Event Subscribe ID:"<<Evnt_SubscribeHandlers[eventcnt].Evnt_SubscribeHandlers.c_str()<<endl;
        cout<< "Subscribed Data:"<<cJSON_Print(out)<<endl;
        if (Evnt_SubscribeHandlers[eventcnt].Evnt_SubscribeHandlers == EventID)
        {
            Evnt_SubscribeHandlers[eventcnt].Evnt_SubscribeData = cJSON_Print(out);
            pthread_cond_signal(&Evnt_Subscribe_cv);
        }
    }
    pthread_mutex_unlock(&Evnt_Subscribe_Mutex);
    cout<< "EmitSubscribedEvent End"<<endl;
}

void BindHttpServer::EmitEvent(std::string EventID, cJSON *out)
{
    // pthread_mutex_lock(&Event_Mutex);
    cout<< "EventID:"<<EventID.c_str()<<endl;

    EmitSubscribedEvent(EventID,out);
    // for (uint8_t eventcnt=0;eventcnt<sseRequestsCnt;eventcnt++)
    // {
    //     cout<< "sseReqName:"<<sseHandlers[eventcnt].sseReqName.c_str()<<endl;
    //     if (sseHandlers[eventcnt].sseReqName == EventID)
    //     {
    //         sseHandlers[eventcnt].EventData = out;
    //         pthread_cond_signal(&Event_cv);
    //     }
    // }
    // pthread_mutex_unlock(&Event_Mutex);
}

std::string GetEventStatus(std::string EventID)
{
    std::string RetVal;

    for (uint8_t eventcnt=0;eventcnt<Evnt_SubscribeRequestsCnt;eventcnt++)
    {
        // cout<< "sseReqName:"<<Evnt_SubscribeHandlers[eventcnt].Evnt_SubscribeHandlers.c_str()<<endl;
        if (Evnt_SubscribeHandlers[eventcnt].Evnt_SubscribeHandlers == EventID)
        {
            RetVal = Evnt_SubscribeHandlers[eventcnt].Evnt_SubscribeData;
            break;
        }
    }
    return (RetVal);
}

void BindHttpServer::SetHandlers(Adapter_interaction_get GetHandleIn,
                                 Adapter_interaction_put PutHandleIn, 
                                 Adapter_thing_get thingGetIn,
                                 Adapter_affordance_get affordIn,
                                 Adapter_interaction_get SubsHandleIn)
{
	ESP_LOGI(LOGGER_COMP_HTTPSERVER,"SetHandlers call");    
    HandleAffordanceGet = GetHandleIn;
    HandleAffordancePut = PutHandleIn;
    HandleThing = thingGetIn;
    HandleAffordance = affordIn;
    HandleSubscription = SubsHandleIn;
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

void BindHttpServer::wifi_init_sta(void)
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

void BindHttpServer::Initialize(unsigned int port)
{
    Port = port;
}

void BindHttpServer::Process(void)
{

}

BindHttpServer::BindHttpServer(void)
{

    // Poco::DNSSD::initializeDNSSD();
    // SocketAddress WifiHotspot_Addr ("192.168.4.1",9999);
    // ServerSocket WifiHotspot_Socket(WifiHotspot_Addr);
    // HTTPServer WifiHotspot(new BindHttpServerHandlerFactory, WifiHotspot_Socket, new HTTPServerParams);    
    // WotServer = new HTTPServer(new BindHttpServerHandlerFactory, ServerSocket(9090), new HTTPServerParams);
    // WotSseServer = new HTTPServer(new BindSseServerHandlerFactory, ServerSocket(9091), new HTTPServerParams);
    // SocketAddress WotServer_Addr ("192.168.137.100",9090);
    // ServerSocket WotServer_Socket(WotServer_Addr);
    // WotServer = new HTTPServer(new BindHttpServerHandlerFactory, WotServer_Socket, new HTTPServerParams);
}

/* An HTTP_ANY handler */
static esp_err_t any_handler(httpd_req_t *req)
{
    Logger* pLogger = NULL; // Create the object pointer for Logger Class
    pLogger = Logger::getInstance();
    // cJSON *Response;
    stringstream ResponseStr;
    std::string RequestMethod = "GET";
    bool ResponseSt;
    bool Subscription;
    std::string Host;
    std::string ThingURI[10];
    std::string Interaction_Type="";
    std::string Interaction_Name="";
    std::string Thing_Name;
    std::string Root_Name;
    unsigned int RequestLevel;
    // Poco::JSON::Object Temp;
    cJSON *Response = cJSON_CreateObject();
    const char *uri = req->uri;
	std::string Request_URI = (const char*)uri;
	// cout << "Request URI=" << req.getURI() << endl;
	RequestLevel = ParseThingsURI(Request_URI,ThingURI);
    Subscription = false;
    switch(RequestLevel)
    {
        // case URI_THINGS_INDEX:
        case URI_THING_INDEX:
        {
            Root_Name = ThingURI[0];
            Thing_Name = ThingURI[0];
            if (RequestMethod == "GET")
            {
                HandleThing(Host,Thing_Name,Root_Name, Response);
                const char* resp_str = cJSON_Print(Response);
                httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
                free(resp_str);
                // End response
                httpd_resp_send_chunk(req, NULL, 0);       
            }
        }
        break;
        case URI_SUBSCRIPTION_INDEX:
        {
            Subscription = true;
        }
        /* Handler for specific Interaction */
        case URI_AFFORDANCE_INDEX:
        {
            Interaction_Name = ThingURI[URI_AFFORDANCE_INDEX]; // Name of the interaction
        }
        /* Handler for reteriving all Interaction */
        case URI_AFFORDANCES_INDEX:
        {
            Root_Name = ThingURI[0];    // Root Name is not used
            Thing_Name = ThingURI[0];
            if (ThingURI[URI_AFFORDANCES_INDEX] == PROPERTY_INTERACT)
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
            pLogger->PrintLog(LOG_LEVEL_DEBUG, LOGGER_COMP_HTTPSERVER,"Interaction Type: ",Interaction_Type.c_str());
            pLogger->PrintLog(LOG_LEVEL_DEBUG, LOGGER_COMP_HTTPSERVER,"Interaction Name: ",Interaction_Name.c_str());
            if (Interaction_Name == "")
            {
                /* Handler for reteriving all Interaction */
                HandleAffordance(Host, Thing_Name, Root_Name, Interaction_Type, Response);
                // Response.stringify(out);
                const char* resp_str = cJSON_Print(Response);
                httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
                free(resp_str);
                // End response
                httpd_resp_send_chunk(req, NULL, 0);
            }
            else
            {
                /* Handler for specific Interaction request*/
                if (RequestMethod == "GET")
                {
                    if (Subscription == false)
                    {
                        /* Data will be updated in ostream out parameter in handler function*/
                        HandleAffordanceGet(Thing_Name, Root_Name, Interaction_Type,Interaction_Name, Response);
                        // Response.stringify(out);
                        /* Send response with body set as the
                        * string passed in user context*/
                        const char* resp_str = cJSON_Print(Response);
                        httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
                        free(resp_str);
                        // End response
                        httpd_resp_send_chunk(req, NULL, 0);
                    }
                    else
                    {
                        Evnt_SubscribeHandlers[Evnt_SubscribeRequestsCnt].Evnt_SubscribeHandlers = Interaction_Name;
                        // sseHandlers[sseRequestsCnt].sseResp = req;
                        HandleSubscription(Thing_Name, Root_Name, Interaction_Type,Interaction_Name, Response);

                        Evnt_SubscribeRequestsCnt++;
                        pthread_mutex_lock(&Evnt_Subscribe_Mutex);
                        pthread_cond_wait(&Evnt_Subscribe_cv, &Evnt_Subscribe_Mutex);
                        std::string ResponseString = GetEventStatus(Interaction_Name);
                        // cJSON_AddStringToObject(Response,"data", ResponseString.c_str());
                        // Response.stringify(out);//
                        // out << EventData;
                        //Response.stringify(out);
                        pthread_mutex_unlock(&Evnt_Subscribe_Mutex);
                        /* Send response with body set as the
                        * string passed in user context*/
                        const char* resp_str = ResponseString.c_str();
                        httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
                        free(resp_str);
                        // End response
                        httpd_resp_send_chunk(req, NULL, 0);
                    }
                }
                else
                {
                    // size_t size = (size_t)req.getContentLength();
                    // cout << "size:" << size <<endl;
                    // std::istream& stream = req.stream();
                    /* Data will be extracted in istream stream paramter in handler function */
                    // HandleAffordancePut(Thing_Name, Root_Name, Interaction_Type,Interaction_Name, stream,size);
                
                    /* To be deleted */
                    // std::string encoded_content;
                    // std::string content;

                    // encoded_content.resize(size);
                    // stream.read(&encoded_content[0], size);
                    // Poco::JSON::Parser parser;
                    // Poco::Dynamic::Var result = parser.parse(encoded_content);
                    // Poco::JSON::Object::Ptr pObject = result.extract<Poco::JSON::Object::Ptr>();
                    // content = pObject->getValue<std::string>("value");

                    // Response.set("status", "success");
                    // cout << endl << "Request received: " << content << endl;
                    // Response.stringify(out);
                }
            }
        }

        break;
    }
    return ESP_OK;
}

static const httpd_uri_t any = {
    .uri       = "/*",
    .method    = HTTP_GET,
    .handler   = any_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = NULL
};

void BindHttpServer::Start(void)
{
    pthread_condattr_t Evnt_SubscribeCV_attr;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;
    config.server_port = 9090;
    config.lru_purge_enable = true;
    // pLogger = Logger::getInstance();

    // //Initialize NVS
    pthread_condattr_init( &Evnt_SubscribeCV_attr);
    pthread_condattr_setclock( &Evnt_SubscribeCV_attr, CLOCK_MONOTONIC);
    pthread_mutex_init(&Evnt_Subscribe_Mutex, NULL);
    pthread_cond_init (&Evnt_Subscribe_cv, NULL);
    wifi_init_sta();
	// pLogger->PrintLog(LOG_LEVEL_DEBUG, LOGGER_COMP_HTTPSERVER,":Initializing");
    // pthread_mutex_lock(&Evnt_Subscribe_Mutex);
    // pthread_cond_wait(&Evnt_Subscribe_cv, &Evnt_Subscribe_Mutex);
    // // Start the httpd server
    // pLogger->PrintLog(LOG_LEVEL_DEBUG, LOGGER_COMP_HTTPSERVER,"Starting server on port:", config.server_port);
    if (httpd_start(&WotServer, &config) == ESP_OK) {
        // Set URI handlers
        // pLogger->PrintLog(LOG_LEVEL_DEBUG, LOGGER_COMP_HTTPSERVER,"Registering URI handlers");
        httpd_register_uri_handler(WotServer, &any);
    }
}

void BindHttpServer::Stop(void)
{
     // Ensure handle is non NULL
     if (WotServer != NULL) {
         // Stop the httpd server
         httpd_stop(WotServer);
     }
}