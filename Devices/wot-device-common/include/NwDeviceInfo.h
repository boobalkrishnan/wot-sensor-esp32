#ifndef NWDEVICEINFO_H
#define NWDEVICEINFO_H

#include <stdint.h>
#include <string>
#include "NwDeviceEnum.h"

using namespace std;



class NwDeviceInfo
{
    public:
        int DeviceID;
        std::string NwDeviceName;
        NwDeviceEnum NwDeviceNetwork;
        NwDeviceVendorEnum NwDeviceVendor;
        NwDeviceProtocolEnum NwDeviceProtocol;
        NwDeviceTypeEnum NwDeviceType;
};

class NwDeviceWiFiInfo: public NwDeviceInfo
{
    public:
        std::string NwDeviceWiFiIp;
        int NwDeviceWiFiSrcPort;
        int NwDeviceWiFiDesPort;    
        std::string NwDeviceWiFiMac;
};

class NwAdapterInfo
{
        NwDeviceEnum NwDeviceNetwork;
        NwDeviceVendorEnum NwDeviceVendor;
        NwDeviceProtocolEnum NwDeviceProtocol;
        NwDeviceTypeEnum NwDeviceType;
        
};
#endif