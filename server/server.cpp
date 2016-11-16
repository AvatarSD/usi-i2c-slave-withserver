/*
 * by S.D.
 * avatarsd.com
 */

#include "server.h"
#include <usiTwiSlave.h>
#include <memory.h>


#define I2C_SLAVE_ADDRESS 0x04

I2CSlaveServer::I2CSlaveServer() : reg_size(memory::mapsize())
{
    device = UsiTwiSlave::getInstance();
    reg_position = 0;
}

void I2CSlaveServer::setup()
{
    address = I2C_SLAVE_ADDRESS; //todo:: temp

    device->init(address);
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

#if TWI_REQUIRE_BUFFERS
/**
 * This is called for each read request we receive, never put more than one byte
 * of data (with TinyWireS.send) to the
 * send-buffer when using this callback
 */
void UsiTwiClientServer::requestEvent()
{
    device->put(i2c_regs[reg_position++]);
    if(reg_position >= reg_size)
        reg_position = 0;
}
#else
int16_t I2CSlaveServer::requestEvent(uint8_t num)
{
    if(reg_position >= reg_size) {
        reg_position = 0;
        return ERR;
    }
    return memory::read(reg_position++);
}
#endif

#if TWI_REQUIRE_BUFFERS
/**
 * The I2C data received -handler
 *
 * This needs to complete before the next incoming transaction (start, data,
 * restart/stop) on the bus does
 * so be quick, set flags for long running tasks to be called from the mainloop
 * instead of running them directly,
 */
void UsiTwiClientServer::receiveEvent(uint8_t howMany)
{
    reg_position = device->get();
    if(reg_position >= reg_size) return;
    while(--howMany) {
        i2c_regs[reg_position++] = device->get();
        if(reg_position >= reg_size)
            reg_position = 0;
    }
}
#else
int8_t I2CSlaveServer::receiveEvent(uint8_t num, uint8_t data)
{
    if(num == 0) {
        if(data < reg_size)
            reg_position = data;
        else
            return ERR;
        return OK;
    }
    if(reg_position >= reg_size) {
        reg_position = 0;
        return ERR;
    }
    memory::write(reg_position++, data);
    return OK;
}
#endif


void I2CSlaveServer::loop()
{
    /**
     * This is the only way we can detect stop condition
     * (http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&p=984716&sid=82e9dc7299a8243b86cf7969dd41b5b5#984716)
     * it needs to be called in a very tight loop in order not to miss any
     * (REMINDER: Do *not* use delay() anywhere, use tws_delay() instead).
     * It will call the function registered via TinyWireS.onReceive(); if there is
     * data in the buffer on stop.
     */
#if TWI_REQUIRE_STOP_CHEK
    device->checkStopAndReceiveCall();
#endif
}
