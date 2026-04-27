#ifndef DEVICEIMP_H
#define DEVICEIMP_H


#include "WoT.h"
#include <stdint.h>
#include <string>
#include <unistd.h>

using namespace std;
enum class DeviceNetworkEnum
{
  Device_WiFi,
  Device_Zigbee,
  Device_Thread,
};

enum class DeviceVendorEnum
{
  DeviceVendor_TpLink,
  DeviceVendor_Custom
};

enum class DeviceProtocolEnum
{
  DeviceProtocol_UDP,
  DeviceProtocol_Zigbee
};

enum DeviceTypeEnum
{
  Device_Plug=0,
  Device_Switch,
  Device_Light,
  Device_Sensor,
  Device_Unknown
};

typedef struct 
{
  enum DeviceTypeEnum DeviceType;
  std::string DeviceTypeStr;
}DeviceTypeString_t;



class DeviceInfo
{
public:
  std::string DeviceID;
  std::string DeviceName;
  DeviceNetworkEnum DeviceNetwork;
  DeviceVendorEnum DeviceVendor;
  DeviceProtocolEnum DeviceProtocol;
  DeviceTypeEnum DeviceType;
  std::string DeviceIP;
  int DeviceSrcPort;
  int DeviceDesPort;
  std::string DeviceMAC;
};

extern void Device_Init (WoT *WoTIn);

// class DeviceImp
// {
//     private:
//         // EspWifi *MyESP;
//       public:
//         pthread_t device_thread;
//         void Discover(void);
//         bool DeviceInitialize(void);
//         void Device_Process(void);
// };

#endif