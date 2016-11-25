/*
 * by S.D.
 * avatarsd.com
 */

#include "server.h"
#include <usiTwiSlave.h>
#include <memory.h>
#include <settings.h>


I2CSlaveServer::I2CSlaveServer() : reg_size(memory::mapsize())
{
    device = UsiTwiSlave::getInstance();
    reg_position = 0;
}

void I2CSlaveServer::setup()
{
    device->init(settingsinternal::getI2cAddress());
    device->onReceiveSetHandler(&receivetIsrVec);
    device->onRequestSetHandler(&requestIsrVec);

}

I2CSlaveServer * I2CSlaveServer::getInstance()
{
    static I2CSlaveServer server;
    return &server;
}

int16_t I2CSlaveServer::requestIsrVec(uint8_t num)
{
    return getInstance()->requestEvent(num);
}

int8_t I2CSlaveServer::receivetIsrVec(uint8_t num, uint8_t data)
{
    return getInstance()->receiveEvent(num, data);
}

int16_t I2CSlaveServer::requestEvent(uint8_t)
{
    if(reg_position >= reg_size)
        return ERR;
    return memory::read(reg_position++);
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
    return memory::write(reg_position++, data);
}

