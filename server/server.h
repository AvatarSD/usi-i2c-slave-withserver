#ifndef I2C_SERVER_H
#define I2C_SERVER_H

#include <inttypes.h>


class UsiTwiSlave;
class Memory;

class I2CSlaveServer
{
public:
    I2CSlaveServer(Memory & memory, UsiTwiSlave & device,
                   uint8_t addr);

    uint8_t getAddress();
    void setAddress(uint8_t addr);

private:
    int8_t receiveEvent(uint8_t num, uint8_t data);
    int16_t requestEvent(uint8_t num);

    Memory * memory;
    UsiTwiSlave * device;

    // Tracks the current register pointer position
    const uint16_t reg_size;
    volatile uint16_t reg_position;

};
#endif
