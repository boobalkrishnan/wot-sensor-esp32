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

#include "BindWebSocket.h"
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
#define MAX_CLIENTS 1

static int s_retry_num = 0;

// HTTPServer *WifiHotspot;
httpd_handle_t  WotServer = NULL;
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
    std::string ThingID;
}Evnt_SubscribeHandlers_t;
unsigned int PortGlobal;
uint32_t GlobalMessageCounter=0;
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
// Async event sender
typedef struct {
    httpd_handle_t hd;
    int fd;
    char payload[256];
} async_resp_arg_t;

static void send_async_event(void *arg) {
    async_resp_arg_t *resp = (async_resp_arg_t *)arg;
    //cout << resp->payload <<endl;
    httpd_ws_frame_t frame = {
        .type = HTTPD_WS_TYPE_TEXT,
        .payload = (uint8_t *)resp->payload,
        .len = strlen(resp->payload)
    };

    esp_err_t ret = httpd_ws_send_frame_async(resp->hd, resp->fd, &frame);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to send async frame to fd %d", resp->fd);
    }

    free(resp);
}

// Broadcast to all WebSocket clients
static void broadcast_event(httpd_handle_t server, cJSON *out) {
    int fds[MAX_CLIENTS];
    size_t clients = MAX_CLIENTS;
    const char *json = cJSON_PrintUnformatted(out);
    //cout<<"broadcast_event"<<json<<endl;
    if (httpd_get_client_list(server, &clients, fds) == ESP_OK) {
        for (size_t i = 0; i < clients; ++i) {
            if (httpd_ws_get_fd_info(server, fds[i]) == HTTPD_WS_CLIENT_WEBSOCKET) {
                async_resp_arg_t *resp = (async_resp_arg_t *)calloc(1, sizeof(async_resp_arg_t));
                resp->hd = server;
                resp->fd = fds[i];
                strncpy(resp->payload, json, sizeof(resp->payload) - 1);
                httpd_queue_work(server, send_async_event, resp);
            }
        }
    }
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

void EmitSubscribedEvent(httpd_handle_t server, std::string EventID, cJSON *out)
{
    cJSON *Response = cJSON_CreateObject();
    for (uint8_t eventcnt=0;eventcnt<Evnt_SubscribeRequestsCnt;eventcnt++)
    {
        // cout<< "sseReqName:"<<Evnt_SubscribeHandlers[eventcnt].Evnt_SubscribeHandlers.c_str()<<endl;
        if (Evnt_SubscribeHandlers[eventcnt].Evnt_SubscribeHandlers == EventID)
        {
            cout<< "EmitSubscribedEvent Start"<<endl;
            cJSON_AddStringToObject(Response, "thingID", Evnt_SubscribeHandlers[eventcnt].ThingID.c_str());
            cJSON_AddNumberToObject(Response, "messageID", GlobalMessageCounter++);
            cJSON_AddStringToObject(Response, "operation", "subscribeevent");
            cJSON_AddStringToObject(Response, "name", EventID.c_str());
            cJSON_AddStringToObject(Response, "messageType", "notification");
            cJSON_AddItemToObject(Response, "value", cJSON_Duplicate(out,1));
            //cout<< "EmitSubscribedEvent:"<<cJSON_Print(Response)<<endl;
            broadcast_event(server, Response);
            cout<< "EmitSubscribedEvent End"<<endl;
            break;
        }
    }
}

void BindWebSocketServer::EmitEvent(std::string EventID, cJSON *out)
{
    // pthread_mutex_lock(&Event_Mutex);
    cout<< "EventID:"<<EventID.c_str()<<endl;

    EmitSubscribedEvent(WotServer, EventID, out);

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

void BindWebSocketServer::wifi_init_sta(void)
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
void BindWebSocketServer::SetHandlers(Adapter_interaction_get GetHandleIn,
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

void BindWebSocketServer::Initialize(unsigned int port)
{
    Port = port;
    PortGlobal = Port;
}

void BindWebSocketServer::Process(void)
{

}

BindWebSocketServer::BindWebSocketServer(void)
{

}

static esp_err_t td_handler(httpd_req_t *req)
{
    std::string Thing_Name;
    std::string Root_Name;
    cJSON *Response = cJSON_CreateObject();
    std::string ThingURI[10];
    const char *uri = req->uri;
	std::string Request_URI = (const char*)uri;
    uint8_t RequestLevel;
    char ip_str[16];
    esp_netif_ip_info_t ip_info;
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    esp_netif_get_ip_info(netif, &ip_info);
    sprintf(ip_str, IPSTR, IP2STR(&ip_info.ip));
    std::string IP(ip_str);

    std::string HostIP = "ws://"+IP+":"+std::to_string(PortGlobal);

	RequestLevel = ParseThingsURI(Request_URI,ThingURI);
    Root_Name = "SensorDevice"; ///ThingURI[0];
    Thing_Name = "SensorDevice"; //ThingURI[0];

    HandleThing(HostIP,Thing_Name,Root_Name, Response);         

    const char* resp_str = cJSON_Print(Response);
    cout<<resp_str<<endl;
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);

    // End response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}
//uint8_t buf[1024];
/* An HTTP_ANY handler */
static esp_err_t ws_handler(httpd_req_t *req)
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
    cJSON *FinalResponse = cJSON_CreateObject();
    ESP_LOGI(TAG, "ws_handler called for: %d", req->method);
    if (req->method == HTTP_GET) return ESP_OK;

    httpd_ws_frame_t ws_pkt;
    uint8_t *buf = NULL;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed to get frame len with %d", ret);
        return ret;
    }
    if (ws_pkt.len)
    {
        buf = (uint8_t*) calloc(1, ws_pkt.len + 1);
        if (buf == NULL)
        {
            ESP_LOGE(TAG, "Failed to calloc memory for buf");
            return ESP_ERR_NO_MEM;
        }
        ws_pkt.payload = buf;
        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
            free(buf);
            return ret;
        }
        ESP_LOGI(TAG, "Got packet with message: %s", ws_pkt.payload);
    }
    cJSON *root = cJSON_Parse((char *)ws_pkt.payload);

    const cJSON *thingID = cJSON_GetObjectItem(root, "thingID");
    const cJSON *messageID = cJSON_GetObjectItem(root, "messageID");
    const cJSON *messageType = cJSON_GetObjectItem(root, "messageType");
    const cJSON *operation = cJSON_GetObjectItem(root, "operation");
    const cJSON *name = cJSON_GetObjectItem(root, "name");
    Thing_Name = thingID->valuestring;
    Root_Name = Thing_Name;
    Interaction_Name = name->valuestring;
    cout<<"Value:"<<Interaction_Name<<endl;
    cJSON_AddItemToObject(Response, "thingID", cJSON_Duplicate(thingID, 1));
    cJSON_AddItemToObject(Response, "messageID", cJSON_Duplicate(messageID, 1));
    cJSON_AddItemToObject(Response, "operation", cJSON_Duplicate(operation, 1));
    cJSON_AddItemToObject(Response, "name", cJSON_Duplicate(name, 1));
    cJSON_AddStringToObject(Response, "messageType", "response");
    if (operation && strcmp(operation->valuestring, "readproperty") == 0)
    {
        Interaction_Type = PROPERTY_INTERACT;
        HandleAffordanceGet(Thing_Name, Root_Name, Interaction_Type,Interaction_Name, Response);
    }
    else if (operation && strcmp(operation->valuestring, "observeproperty") == 0)
    {
        Interaction_Type = PROPERTY_INTERACT;
        Evnt_SubscribeHandlers[Evnt_SubscribeRequestsCnt].Evnt_SubscribeHandlers = Interaction_Name;
        Evnt_SubscribeHandlers[Evnt_SubscribeRequestsCnt].ThingID = thingID->valuestring;
        HandleSubscription(Thing_Name, Root_Name, Interaction_Type,Interaction_Name, Response);
        Evnt_SubscribeRequestsCnt++;
    } else if (operation && strcmp(operation->valuestring, "subscribeevent") == 0)
    {
        Interaction_Type = EVENT_INTERACT;
        Evnt_SubscribeHandlers[Evnt_SubscribeRequestsCnt].Evnt_SubscribeHandlers = Interaction_Name;
        Evnt_SubscribeHandlers[Evnt_SubscribeRequestsCnt].ThingID = thingID->valuestring;;
        HandleSubscription(Thing_Name, Root_Name, Interaction_Type,Interaction_Name, Response);
        Evnt_SubscribeRequestsCnt++;
    } else {
        cJSON *error = cJSON_CreateObject();
        cJSON_AddNumberToObject(error, "code", -32601);
        cJSON_AddStringToObject(error, "message", "Method not found");
        cJSON_AddItemToObject(Response, "error", error);
    }

    broadcast_event(req->handle, Response);
    // char *out = cJSON_PrintUnformatted(Response);
    // httpd_ws_frame_t out_frame = {
    //     .type = HTTPD_WS_TYPE_TEXT,
    //     .payload = (uint8_t *)out,
    //     .len = strlen(out)
    // };
    // httpd_ws_send_frame(req, &out_frame);

    cJSON_Delete(root);
    cJSON_Delete(Response);
    // free(out);
    return ESP_OK;
}

static httpd_uri_t td_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = td_handler,
    .user_ctx  = NULL
};

static httpd_uri_t ws_uri = {
    .uri       = "/ws",
    .method    = HTTP_GET,
    .handler   = ws_handler,
    .user_ctx = NULL,
    .is_websocket = true,
    .supported_subprotocol = "webthingprotocol"
};


void BindWebSocketServer::Start(void)
{
    pthread_condattr_t Evnt_SubscribeCV_attr;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;
    config.server_port = 9090;
    config.lru_purge_enable = true;
    config.stack_size = 8192;
    pthread_condattr_init( &Evnt_SubscribeCV_attr);
    pthread_condattr_setclock( &Evnt_SubscribeCV_attr, CLOCK_MONOTONIC);
    pthread_mutex_init(&Evnt_Subscribe_Mutex, NULL);
    pthread_cond_init (&Evnt_Subscribe_cv, NULL);
    wifi_init_sta();

    if (httpd_start(&WotServer, &config) == ESP_OK) 
    {
        // httpd_register_uri_handler(WotServer, &any);
        httpd_register_uri_handler(WotServer, &td_uri);
        httpd_register_uri_handler(WotServer, &ws_uri);
        ESP_LOGI(TAG, "HTTP server started");

    }
}

void BindWebSocketServer::Stop(void)
{
     // Ensure handle is non NULL
     if (WotServer != NULL) {
         // Stop the httpd server
         httpd_stop(WotServer);
     }
}