
#ifndef NWADAPTER_H
#define NWADAPTER_H
#include "NwDeviceInfo.h"

typedef enum
{
    UnInitialized=0,
    Initialized,
    Active,
    Error
}NwAdapterStatus_t;


class NwAdapter
{
    /**
     * @class NwAdapter 
     *
     * @brief Base class for Network Adapters
     */    
    public:
        std::string NwAdapterName; ///< Network Adapter's Name
        std::string NwAdapterProtocol; ///< Network Adapter's Protocol
        NwAdapterStatus_t NwAdapterStatus; ///< Network Adapter's Status    public:
        /**
            * A member fucntion to initialize the class with adapter name and adapter protocol.
            * @param AdapterName - Name of Adapter.
            * @param AdapterProtocol - Name of the adapter protocol.
            * @return adapter Initialization status <NwAdapterStatus_t>.
            */
        virtual NwAdapterStatus_t Initialize(std::string AdapterName,std::string AdapterProtocol);
        /**
            * A member fucntion to initialize the devices connected via the adapter.
            * @param connected device information.
            * @param number of devices.
            * @return adapter Initialization status.
            */        
        virtual void DevicesInit(NwDeviceInfo *DevicesCached, int NoOfDevices);
        virtual void DeviceDiscovery(int DiscoveryTimeout);

        virtual void Send(NwDeviceInfo *DevicesCached,char *SendData, int SendDataLen);
        virtual int Receive(NwDeviceInfo *DevicesCached,char *ReceiveData);        
        /* Return type is property value*/
        virtual int GetProperty(NwDeviceInfo *DevicesCached,int PropertyID);
        virtual void SetProperty(NwDeviceInfo *DevicesCached,int PropertyID, int PropertyValue);        
};

#endif