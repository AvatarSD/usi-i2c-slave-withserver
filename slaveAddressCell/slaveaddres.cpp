#include "slaveaddres.h"

int16_t SlaveAddress::newAddr = ERR;
ISlaveAddress * SlaveAddress::iaddress;

Error SlaveAddress::write(Address addr, uint8_t data, Num num)
{
    newAddr = data;
    return OK;
}

ReadType SlaveAddress::read(Address addr, Num num)
{
    if(!iaddress) {
        if(newAddr != ERR)
            iaddress->setAddress(newAddr);
        newAddr = ERR;
        return iaddress->getAddress();
    }
    newAddr = ERR;
    return ERR;
}

void SlaveAddress::setISlaveAddress(ISlaveAddress * iaddress)
{
    iaddress = iaddress;
}

ISlaveAddress * SlaveAddress::getISlaveAddress()
{
    return iaddress;
}
