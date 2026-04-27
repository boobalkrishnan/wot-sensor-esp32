// /**
// * @file
// * @author Boobalakrishnan
// * @version 1.0
// *
// * @section LICENSE
// *
// *
// * @section DESCRIPTION
// *
// * Main application file of WoT
// *
// */

#include <stdint.h>
#include <unistd.h>
#include "AppMain.h"
#include "Logger.h"

#include "WoTServient.h"

#include "BindServer.h"
#include "BindClient.h"
#include "DeviceImp.h"

#include "WoT.h"
#include "WoTServient.h"

using namespace std;
using namespace Logging;
pthread_t Http_Thread;
pthread_mutex_t Http_Mutex;
WoT MyWoT;


BindServer MyHttpServer(BIND_HTTP_SERVER);
BindServer MyCoAPServer(BIND_COAP_SERVER);


BindClient MyClient(BIND_WS_CLIENT);

WoTServient Myservient;

void serial_check() {

};

void *
Http_Process(void *)
{
    for (;;)
    {
        // Myservient.Process();
        // sleep(0.2);
    }
}

int defmain()
{
    // Log message C++ Interface
    Logger *pLogger = NULL; // Create the object pointer for Logger Class
    pLogger = Logger::getInstance();
    pLogger->updateLogLevel(LOG_LEVEL_DEBUG);
    pLogger->updateLogType(CONSOLE);
    Myservient.Initialize(9090, "room1", false, "None");
    pLogger->debug("Starting");

    MyHttpServer.Initialize(9090);
    MyCoAPServer.Initialize(5683);

    Myservient.AddServer(&MyHttpServer);
    Myservient.AddServer(&MyCoAPServer);

    Myservient.Start();

    MyWoT.AddServient(&Myservient);

    Device_Init(&MyWoT);

    int count = 10;

    if (pthread_mutex_init(&Http_Mutex, NULL) != 0)
    {
        printf("\n mutex init has failed\n");
    }

    for (;;)
    {
        MyCoAPServer.Process();
        Myservient.Process();
        sleep(10);
    }
    return 0;
};

int AppMain(int argc, char **argv)
{
    defmain();
};
