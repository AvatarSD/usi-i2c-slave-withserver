#ifndef USI_H
#define USI_H

#include <inttypes.h>

enum WireMode {
    NORMAL = 0b00,
    SPI = 0b01,
    TWI = 0b10,
    TWI_WAIT = 0b11
};

enum ClockMode {
    NONE = 0b000,
    SOFTWARE = 0b001,
    TIMER0_CMP = 0b010,
    EXT_POS = 0b100, // for i2c slave
    EXT_NEG = 0b110,
    EXT_POS_SOFT_CONT = 0b101,
    EXT_NEG_SOFT_CONT = 0b111
};


class iUSIcallback
{
public:
    virtual void startConditionHandler();
    virtual void overflowHandler();
};


class USI
{
public:
    void setIsrHandler(iUSIcallback * handler)
    {
        usiIsr = handler;
    }

    void isrOvfl();
    void isrStrt();

    //data
    volatile uint8_t & data;

    //buffer
    volatile uint8_t & buffer;

    //status
    void setStatus(bool start,
                   bool ovf,
                   bool stop,
                   bool collision,
                   uint8_t counter);

    bool getStartCondIntFlag();
    void clearStartCondIntFlag();
    void releaseStartCondLock();

    bool getOvfIntFlag();
    void clearOvfIntFlag();
    void releaseOvfCondLock();

    bool getStopCondIntFlag();
    void clearStopCondIntFlag();

    bool haveCollision();
    void clearCollisionFlag();

    uint8_t counterValue();
    void setCounterValue(uint8_t val);

    //control register
    void enableStartInt();
    void disableStartInt();

    void enableOvfInt();
    void disableOvfInt();

    void setWireMode(WireMode);
    WireMode getWireMode();

    void setClockMode(ClockMode);
    ClockMode getClockMode();

    void shiftClockStrobeCounter();
    void toggleClockPortPin();

    //direct accses to pin's
    void enableSDAOpenDrain();
    void disableSDAOpenDrain();
    bool getSDAState();
    void enableForceHoldSDA();
    void disableForceHoldSDA();
    void enableSCLOpenDrain();
    void disableSCLOpenDrain();
    bool getSCLState();
    void enableForceHoldSCL();
    void disableForceHoldSCL();

    static USI * instance()
    {
        static USI usi;
        return &usi;
    }

private:
    USI();
    USI(const USI &) = delete;
    USI & operator=(const USI &) = delete;

    //isr's
    iUSIcallback * usiIsr = 0;
};

#endif
