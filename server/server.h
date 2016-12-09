#ifndef I2C_SERVER_H
#define I2C_SERVER_H

#include <inttypes.h>
#include "../I2C/usiTwiSlave.h"


class IMappedMemory
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



class I2CSlaveServer : public IServer
{
public:
    I2CSlaveServer(ITwiSlave * net, IMappedMemory * memory);

    IMappedMemory * getMemoryObject();
    ITwiSlave * getNetworkObject();

private:
    int8_t onReceiver(uint8_t num, uint8_t data) override;
    int16_t onRequest(uint8_t num)  override;

    IMappedMemory * memory;
    ITwiSlave * network;

    // Tracks the current register pointer position
    const uint16_t reg_size;
    volatile uint16_t reg_position;

};
#endif
