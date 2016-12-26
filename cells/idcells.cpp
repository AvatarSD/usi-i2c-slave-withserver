#include "idcells.h"


GUID::GUID(ISettingsGeneral * common) : common(common) {}

Error GUID::write(Address addr, uint8_t data, Num num)
{
    return OK;
}

ReadType GUID::read(Address addr, Num num)
{
    return common->getDeviceGUID(addr);
}

size_t GUID::size()
{
    return GUID_SIZE;
}

DeviceName::DeviceName(ISettingsGeneral * common) : common(common) {}

Error DeviceName::write(Address addr, uint8_t data, Num num)
{
    return OK;
}

ReadType DeviceName::read(Address addr, Num num)
{
    return common->getDeviceName(addr);
}

size_t DeviceName::size()
{
    return DEVNAME_SIZE;
}

DeviceSWver::DeviceSWver(ISettingsGeneral * common) : common(common) {}

Error DeviceSWver::write(Address addr, uint8_t data, Num num)
{
    return OK;
}

ReadType DeviceSWver::read(Address addr, Num num)
{
    return common->getDeviceSWver(addr);
}

size_t DeviceSWver::size()
{
    return sizeof(uint16_t);
}

DeviceHWver::DeviceHWver(ISettingsGeneral * common) : common(common) {}

Error DeviceHWver::write(Address addr, uint8_t data, Num num)
{
    return OK;
}

ReadType DeviceHWver::read(Address addr, Num num)
{
    return common->getDeviceHWver(addr);
}

size_t DeviceHWver::size()
{
    return sizeof(uint16_t);
}

CommonShared::CommonShared(ISettingsGeneral * settings) :
    Composite(&guid, &devname, &devsw, &devhw, &addr),
    guid(settings), devname(settings), devsw(settings), devhw(settings)

{}

void CommonShared::setNetworkObject(ISlaveAddress * netIface)
{
    addr.setNetworkObject(netIface);
}
