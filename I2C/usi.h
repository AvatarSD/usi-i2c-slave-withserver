#ifndef USI_H
#define USI_H

#include <inttypes.h>

class USI
{
public:
    //data
    static volatile uint8_t & data;

    //buffer
    static volatile uint8_t & buffer;

    //status
    static void setStatus(bool start,
                          bool ovf,
                          bool stop,
                          bool collision,
                          uint8_t counter);

    static bool getStartCondIntFlag();
    static void clearStartCondIntFlag();
    static void releaseStartCondLock();

    static bool getOvfIntFlag();
    static void clearOvfIntFlag();
    static void releaseOvfCondLock();

    static bool getStopCondIntFlag();
    static void clearStopCondIntFlag();

    static bool haveCollision();
    static void clearCollisionFlag();

    static uint8_t counterValue();
    static void setCounterValue(uint8_t val);

    //control register
    static void enableStartInt();
    static void disableStartInt();

    static void enableOvfInt();
    static void disableOvfInt();

    enum WireMode
    {
        NORMAL = 0b00,
        SPI = 0b01,
        TWI = 0b10,
        TWI_WAIT = 0b11
    };
    static void setWireMode(USI::WireMode mode);
    static USI::WireMode getWireMode();

    enum ClockMode
    {
        NONE = 0b000,
        SOFTWARE = 0b001,
        TIMER0_CMP = 0b010,
        EXT_POS = 0b100, // for i2c slave
        EXT_NEG = 0b110,
        EXT_POS_SOFT_CONT = 0b101,
        EXT_NEG_SOFT_CONT = 0b111
    };
    static void setClockMode(USI::ClockMode mode);
    static USI::ClockMode getClockMode();

    static void shiftClockStrobeCounter();
    static void toggleClockPortPin();

    //direct accses to pin's
    static void holdDataLine();
    static void releaseDataLine();
    static void holdClockLine();
    static void releaseClockLine();

private:
    USI() {}
    USI(const USI &) {}
    USI & operator=(const USI &) = delete;
};

#endif
