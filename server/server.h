#ifndef I2C_SERVER_H
#define I2C_SERVER_H

#include <inttypes.h>
#include "usiTwiSlave.h"
#include <polymorphmemory.h>

class IMemory
{
public:
    virtual int8_t write(uint8_t addr, uint8_t data);
    virtual int16_t read(uint8_t addr);
    virtual uint16_t size();
};

class I2CSlaveServer : public IServer
{
public:
    I2CSlaveServer(IMemory * memory);

    IMemory * getMemoryObject();

private:
    int8_t onReceiver(uint8_t num, uint8_t data) override;
    int16_t onRequest(uint8_t num)  override;

    IMemory * memory;

    // Tracks the current register pointer position
    const uint16_t reg_size;
    volatile uint16_t reg_position;

};
#endif
