#include <pthread.h>
#include <stdint.h>
#include <unistd.h>
// #include "Serial.h"
// #include "EspDrv.h"
#include "Logger.h"
// #include "WoTAdapter.h"
#include "DeviceImp.h"
#include "Virtual_Clk_device.h"

#include "WoT.h"
#include "WoTAction.h"
#include "WoTEvent.h"
#include "WoTProperty.h"
#include "WoTServient.h"
#include "WoTTd.h"
#include "WoTThing.h"


#define WOT_REALTIME_MEASUREMENTS

bool DefaultValue = 0;


pthread_t device_thread;
DeviceInfo *DevicesList[2];

unsigned int DeviceIndex;
ConsumedThing *SensorDeviceIn;

void *Device_Main(void *)
{
    ThingInteractionValue *PropertyValue = new ThingInteractionValue();
    uint8_t TempReadCount = 0;
    while (1)
    {
        sleep(5);
    }
}

void Device_Init(WoT *WoTIn)
{
    // SensorDeviceIn = WoTIn->consume(SensorDevice.dump());
    // SensorDeviceIn->SetEventObserveCallback(EventCallback);
    // SensorDeviceIn->SetClientProtocol(BIND_WS_CLIENT);
    // SensorDeviceIn->SubscribeEvent("motion");

    // WoTIn->produce(HueThingModel.dump());
#ifdef WOT_REALTIME_MEASUREMENTS


#endif
    pthread_create(&device_thread, NULL, &Device_Main, NULL);
    pthread_setname_np(device_thread, "device_thread");
    DevicesList[DeviceIndex++] = VirtualDevice_Init(WoTIn);
    // (void) pthread_join(device_thread, NULL);
}
