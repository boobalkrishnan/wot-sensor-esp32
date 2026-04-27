#ifndef NWDEVICEENUM_H
#define NWDEVICEENUM_H

enum class NwDeviceEnum
{
    NwDevice_WiFi,
    NwDevice_Zigbee,
    NwDevice_Thread,
};

enum class NwDeviceVendorEnum
{
    NwDeviceVendor_TpLink,
    NwDeviceVendor_Custom
};

enum class NwDeviceProtocolEnum
{
    NwDeviceProtocol_UDP,
    NwDeviceProtocol_Zigbee
};

enum class NwDeviceTypeEnum
{
    NwDevice_Plug,
    NwDevice_Switch,
    NwDevice_Light
};
#endif