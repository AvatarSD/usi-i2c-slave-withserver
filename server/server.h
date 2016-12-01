#ifndef I2C_SERVER_H
#define I2C_SERVER_H

#include <inttypes.h>
#include "../I2C/usiTwiSlave.h"


class iMappedMemory
{
public:
    virtual int8_t write(uint8_t addr, uint8_t data)
    {
        return ERR;
    }
    virtual int16_t read(uint8_t addr)
    {
        return ERR;
    }
    virtual uint16_t mapsize()
    {
        return ERR;
    }
};



class I2CSlaveServer : public iServer
{
public:
    I2CSlaveServer(UsiTwiSlave * net, iMappedMemory * memory);

    iMappedMemory * getMemoryObject();
    UsiTwiSlave * getNetworkObject();

private:
    int8_t onReceiver(uint8_t num, uint8_t data) final;
    int16_t onRequest(uint8_t num) final;

    iMappedMemory * memory;
    UsiTwiSlave * network;

    // Tracks the current register pointer position
    const uint16_t reg_size;
    volatile uint16_t reg_position;

};
#endif
