/*
 * by S.D.
 * avatarsd.com
 */

#include "server.h"
#include <usiTwiSlave.h>
#include <memory.h>
#include <settings.h>


I2CSlaveServer::I2CSlaveServer(Memory & memory, UsiTwiSlave & device,
                               uint8_t addr) :
    memory(&memory),
    device(&device),
    reg_size(memory.mapsize()), reg_position(0)
{
    this->device->init(addr);
    //this->device->onReceiveSetHandler(&this->receivetIsrVec); // TODO
    //this->device->onRequestSetHandler(&this->requestIsrVec);
}


uint8_t I2CSlaveServer::getAddress()
{
    return device->getAddress();
}

void I2CSlaveServer::setAddress(uint8_t addr)
{
    device->setAddress(addr);
}


// instance handlers
int16_t I2CSlaveServer::requestEvent(uint8_t)
{
    if(reg_position >= reg_size)
        return ERR;
    return memory->read(reg_position++);
}
int8_t I2CSlaveServer::receiveEvent(uint8_t num, uint8_t data)
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
