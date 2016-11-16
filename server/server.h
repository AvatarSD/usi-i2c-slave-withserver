#ifndef I2C_SERVER_H
#define I2C_SERVER_H

#include <inttypes.h>

class UsiTwiSlave;
class I2CSlaveServer
{
public:
    void setup();
    void loop(); // not used
private:
    int8_t receiveEvent(uint8_t num, uint8_t data);
    int16_t requestEvent(uint8_t num);

    // Tracks the current register pointer position
    volatile uint16_t reg_position;
    const uint16_t reg_size;

    UsiTwiSlave * device;
    uint8_t address;

    I2CSlaveServer();
    I2CSlaveServer(const I2CSlaveServer &) = default;
    I2CSlaveServer & operator=(I2CSlaveServer &) = default;

public:
    static I2CSlaveServer * getInstance();
    static int16_t requestIsrVec(uint8_t num);
    static int8_t receivetIsrVec(uint8_t num, uint8_t data);

};
#endif
