#ifndef IDCELLS_H
#define IDCELLS_H

#include <slaveaddres.h>

#define GUID_SIZE 16
#define DEVNAME_SIZE 4

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
    virtual uint8_t getDeviceGUID(uint8_t pos) const;
    virtual uint8_t getDeviceName(uint8_t pos) const;
    virtual uint8_t getDeviceSWver(uint8_t pos) const;
    virtual uint8_t getDeviceHWver(uint8_t pos) const;
};

class GUID : public IMemory
{
public:
    GUID(ISettingsGeneral * common) : common(common) {}

    Error write(Address addr, uint8_t data, Num num) final {
        return OK;
    }
    ReadType read(Address addr, Num num) final {
        return common->getDeviceGUID(addr);
    }
    size_t size() final {
        return GUID_SIZE;
    }
private:
    ISettingsGeneral * common;
};
class DeviceName : public IMemory
{
public:
    DeviceName(ISettingsGeneral * common) : common(common) {}
    Error write(Address addr, uint8_t data, Num num) final {
        return OK;
    }
    ReadType read(Address addr, Num num = 0) final {
        return common->getDeviceName(addr);
    }
    size_t size() final {
        return DEVNAME_SIZE;
    }
private:
    ISettingsGeneral * common;
};
class DeviceSWver : public IMemory
{
public:
    DeviceSWver(ISettingsGeneral * common) : common(common) {}
    Error write(Address addr, uint8_t data, Num num) final {
        return OK;
    }
    ReadType read(Address addr, Num num = 0) final {
        return common->getDeviceSWver(addr);
    }
    size_t size() final {
        return sizeof(uint16_t);
    }
private:
    ISettingsGeneral * common;
};
class DeviceHWver : public IMemory
{
public:
    DeviceHWver(ISettingsGeneral * common) : common(common) {}
    Error write(Address addr, uint8_t data, Num num) final {
        return OK;
    }
    ReadType read(Address addr, Num num = 0) final {
        return common->getDeviceHWver(addr);
    }
    size_t size() final {
        return sizeof(uint16_t);
    }
private:
    ISettingsGeneral * common;
};

class CommonShared : public
    Composite<GUID, DeviceName, DeviceSWver, DeviceHWver, SlaveAddress>
{
public:
    CommonShared(ISettingsGeneral * settings) :
        Composite(&guid, &devname, &devsw, &devhw, &addr),
        guid(settings), devname(settings), devsw(settings), devhw(settings)

    {}
    void setNetworkObject(ISlaveAddress * netIface)
    {
        addr.setNetworkObject(netIface);
    }

private:
    GUID  guid;
    DeviceName devname;
    DeviceSWver devsw;
    DeviceHWver devhw;
    SlaveAddress addr;
};


#endif // IDCELLS_H
