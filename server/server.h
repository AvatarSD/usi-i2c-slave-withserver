#ifndef I2C_SERVER_H
#define I2C_SERVER_H

#include <inttypes.h>


class UsiTwiSlave;

class I2CSlaveServer
{
public:
    void setup(uint8_t addr);
    uint8_t getAddress();
    void setAddress(uint8_t addr);

private:
    int8_t receiveEvent(uint8_t num, uint8_t data);
    int16_t requestEvent(uint8_t num);

    // Tracks the current register pointer position
    volatile uint16_t reg_position;
    const uint16_t reg_size;

    UsiTwiSlave * device;

    I2CSlaveServer();
    I2CSlaveServer(const I2CSlaveServer &) = default;
    I2CSlaveServer & operator=(I2CSlaveServer &) = default;

public:
    static I2CSlaveServer * getInstance();
    static int16_t requestIsrVec(uint8_t num);
    static int8_t receivetIsrVec(uint8_t num, uint8_t data);

};
#endif
