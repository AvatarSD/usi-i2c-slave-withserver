/*
 * by S.D.
 * avatarsd.com
 */

#include "server.h"
#include <memory.h>


I2CSlaveServer::I2CSlaveServer(iMappedMemory * memory, UsiTwiSlave * device,
                               uint8_t addr, uint8_t multicastaddr) :
    memory(memory), device(device),
    reg_size(memory->mapsize()), reg_position(0)
{
    this->device->init(this, addr, multicastaddr);

}


uint8_t I2CSlaveServer::getAddress()
{
    return device->getAddress();
}

void I2CSlaveServer::setAddress(uint8_t addr)
{
    device->setAddress(addr);
}


int16_t I2CSlaveServer::onRequest(uint8_t)
{
    if(reg_position >= reg_size)
        return ERR;
    return memory->read(reg_position++);
}
int8_t I2CSlaveServer::onReceiver(uint8_t num, uint8_t data)
{
    if(num == 0) {
        if(data < reg_size)
            reg_position = data;
        else
            return ERR;
        return OK;
    }
    if(reg_position >= reg_size)
        return ERR;
    return memory->write(reg_position++, data);
}
