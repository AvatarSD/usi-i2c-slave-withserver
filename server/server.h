#ifndef I2C_SERVER_H
#define I2C_SERVER_H

#include <inttypes.h>
#include <usiTwiSlave.h>


class iMappedMemory
{
public:
    virtual int8_t write(uint8_t addr, uint8_t data) = 0;
    virtual int16_t read(uint8_t addr) = 0;
    virtual uint16_t mapsize() = 0;
};



class I2CSlaveServer : public iServer
{
public:
    I2CSlaveServer(iMappedMemory * memory, UsiTwiSlave * device,
                   uint8_t addr, uint8_t multicastaddr);

    uint8_t getAddress();
    void setAddress(uint8_t addr);

private:
    int8_t onReceiver(uint8_t num, uint8_t data) final;
    int16_t onRequest(uint8_t num) final;

    iMappedMemory * memory;
    UsiTwiSlave * device;

    // Tracks the current register pointer position
    const uint16_t reg_size;
    volatile uint16_t reg_position;

};
#endif
