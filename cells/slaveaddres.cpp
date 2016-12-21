#include "slaveaddres.h"

int16_t SlaveAddress::newAddr = ERR;
ISlaveAddress * SlaveAddress::iaddress = nullptr;

Error SlaveAddress::write(Address addr, uint8_t data, Num num)
{
    newAddr = data;
    return OK;
}

ReadType SlaveAddress::read(Address addr, Num num)
{
    if(iaddress != nullptr) {
        if(newAddr != ERR)
            iaddress->setAddress(newAddr);
        newAddr = ERR;
        return iaddress->getAddress();
    }
    newAddr = ERR;
    return ERR;
}

void SlaveAddress::setNetworkIface(ISlaveAddress * iAddress)
{
    iaddress = iAddress;
}
