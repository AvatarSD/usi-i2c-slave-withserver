#ifndef IDCELLS_H
#define IDCELLS_H

#include <slaveaddres.h>

#define GUID_SIZE 16
#define DEVNAME_SIZE 4

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
    GUID(ISettingsGeneral * common);

    Error write(Address addr, uint8_t data, Num num) final;
    ReadType read(Address addr, Num num) final;
    size_t size() final;
private:
    ISettingsGeneral * common;
};
class DeviceName : public IMemory
{
public:
    DeviceName(ISettingsGeneral * common);
    Error write(Address addr, uint8_t data, Num num) final;
    ReadType read(Address addr, Num num = 0) final;
    size_t size() final;
private:
    ISettingsGeneral * common;
};
class DeviceSWver : public IMemory
{
public:
    DeviceSWver(ISettingsGeneral * common);
    Error write(Address addr, uint8_t data, Num num) final;
    ReadType read(Address addr, Num num = 0) final;
    size_t size() final;
private:
    ISettingsGeneral * common;
};
class DeviceHWver : public IMemory
{
public:
    DeviceHWver(ISettingsGeneral * common);
    Error write(Address addr, uint8_t data, Num num) final;
    ReadType read(Address addr, Num num = 0) final;
    size_t size() final;
private:
    ISettingsGeneral * common;
};

class CommonShared :
    public Composite<GUID, DeviceName, DeviceSWver, DeviceHWver, SlaveAddress>
{
public:
    CommonShared(ISettingsGeneral * settings);
    void setNetworkObject(ISlaveAddress * netIface);

private:
    GUID  guid;
    DeviceName devname;
    DeviceSWver devsw;
    DeviceHWver devhw;
    SlaveAddress addr;
};


#endif // IDCELLS_H
