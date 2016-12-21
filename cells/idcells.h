#ifndef IDCELLS_H
#define IDCELLS_H

#include <slaveaddres.h>


#define GUID_SIZE 16
#define DEVNAME_SIZE 4
#define DEV_NAME {'a', 'h', 't', 'r'}
#define DEV_SW_VER VER(1, 5)
#define DEV_HW_VER VER(0, 1)


#define readByte(word, addr) ((uint8_t)((word >> (addr * 8)) & 0xFF));

template<typename TypeSize>
int8_t writeWord(TypeSize & staticReg, uint8_t addr, uint8_t data)
{
    if(addr == 0) staticReg = 0;
    staticReg |= (TypeSize)(data << (addr * 8));
    if(addr == sizeof(staticReg) - 1) return OK;
    return ERR;
}


class ISettingsGeneral
{
public:
    //ISettingsGeneral
    virtual uint8_t getDeviceGUID(uint8_t pos) const;
    virtual uint8_t getDeviceName(uint8_t pos) const;
    virtual uint8_t getDeviceSWver(uint8_t pos) const;
    virtual uint8_t getDeviceHWver(uint8_t pos) const;
};

extern ISettingsGeneral * common;

class GUID : public Composite<uint8_t[GUID_SIZE]>
{
public:
    static Error write(Address addr, uint8_t data, Num num)
    {
        return OK;
    }
    static ReadType read(Address addr, Num num)
    {
        return common->getDeviceGUID(addr);
    }
};
class DeviceName : public Composite<uint8_t[DEVNAME_SIZE]>
{
public:
    static Error write(Address addr, uint8_t data, Num num)
    {
        return OK;
    }
    static ReadType read(Address addr, Num num = 0)
    {
        return common->getDeviceName(addr);
    }
};
class DeviceSWver : public Composite<uint16_t>
{
public:
    static Error write(Address addr, uint8_t data, Num num)
    {
        return OK;
    }
    static ReadType read(Address addr, Num num = 0)
    {
        return common->getDeviceSWver(addr);
    }
};
class DeviceHWver : public Composite<uint16_t>
{
public:
    static Error write(Address addr, uint8_t data, Num num)
    {
        return OK;
    }
    static ReadType read(Address addr, Num num = 0)
    {
        return common->getDeviceHWver(addr);
    }
};

class CommonShared : public
    Composite<GUID, DeviceName, DeviceSWver, DeviceHWver, SlaveAddress>
{
public:
    static void setSettings(ISettingsGeneral * settings, ISlaveAddress * netIface)
    {
        common = settings;
        SlaveAddress::setNetworkIface(netIface);
    }
};


#endif // IDCELLS_H
