/*
 * by S.D.
 * avatarsd.com
 */

#include "server.h"


I2CSlaveServer::I2CSlaveServer(IMemory * memory) :
    memory(memory), reg_size(memory->size()), reg_position(0)
{

}

IMemory * I2CSlaveServer::getMemoryObject()
{
    return memory;
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
