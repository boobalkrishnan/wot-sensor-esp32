#include "NwAdapter.h"

NwAdapterStatus_t NwAdapter::Initialize(std::string AdapterName,std::string AdapterProtocol)
{

};

void NwAdapter::DevicesInit(NwDeviceInfo *DevicesCached, int NoOfDevices)
{

};
void NwAdapter::DeviceDiscovery(int DiscoveryTimeout)
{

};

int NwAdapter::Receive(NwDeviceInfo *DevicesCached,char *ReceiveData)
{

};
void NwAdapter::Send(NwDeviceInfo *DevicesCached,char *SendData, int SendDataLen)
{

};

int NwAdapter::GetProperty(NwDeviceInfo *DevicesCached,int PropertyID)
{

};

void NwAdapter::SetProperty(NwDeviceInfo *DevicesCached,int PropertyID, int PropertyValue)
{

};
