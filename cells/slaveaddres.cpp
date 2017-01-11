#include "slaveaddres.h"

SlaveAddress::SlaveAddress() : newAddr(ERR), iaddress(nullptr) {}

SlaveAddress::SlaveAddress(ISlaveAddress * iaddress) : newAddr(ERR),
    iaddress(iaddress) {}

Error SlaveAddress::write(Address addr, uint8_t data, Num num)
{
    if(iaddress == nullptr) {
        newAddr = ERR;
        return ERR;
    }
    if(iaddress->getMulticastAddress() == data) {
        iaddress->setAddress(newAddr);
        newAddr = ERR;
    } else
        newAddr = data;
    return OK;
}

ReadType SlaveAddress::read(Address addr, Num num)
{
    if(iaddress == nullptr) {
        newAddr = ERR;
        return ERR;
    }
    if(newAddr != ERR)
        iaddress->setAddress(newAddr);
    newAddr = ERR;
    return iaddress->getAddress();
}

size_t SlaveAddress::size()
{
    return sizeof(uint8_t);
}

void SlaveAddress::setNetworkObject(ISlaveAddress * iAddress)
{
    iaddress = iAddress;
}
