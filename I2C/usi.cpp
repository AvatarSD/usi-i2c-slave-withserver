#include "usi.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "pindefs.h"



//status regs:
void USI::isrOvfl()
{
    if(USI::instance()->usiIsr)
        USI::instance()->usiIsr->overflowHandler();
}

void USI::isrStrt()
{
    if(USI::instance()->usiIsr)
        USI::instance()->usiIsr->startConditionHandler();
}

void USI::setStatus(bool start, bool ovf, bool stop, bool collision,
                    uint8_t counter)
{
    USISR = (start << USISIF)
            | (ovf << USIOIF)
            | (stop << USIPF)
            | (collision << USIDC)
            | ((counter & 0b1111) << USICNT0);
}

bool USI::getStartCondIntFlag()
{
    return USISR & _BV(USISIF);
}

void USI::clearStartCondIntFlag()
{
    USISR |= _BV(USISIF);
}

void USI::releaseStartCondLock()
{
    USISR &= ~_BV(USISIF);
}

bool USI::getOvfIntFlag()
{
    return USISR & _BV(USIOIF);
}

void USI::clearOvfIntFlag()
{
    USISR |= _BV(USIOIF);
}

void USI::releaseOvfCondLock()
{
    USISR &= ~_BV(USIOIF);
}

bool USI::getStopCondIntFlag()
{
    return USISR & _BV(USIPF);
}

void USI::clearStopCondIntFlag()
{
    USISR |= _BV(USIPF);
}

bool USI::haveCollision()
{
    return USISR & _BV(USIDC);
}

void USI::clearCollisionFlag()
{
    USISR |= _BV(USIDC);
}

uint8_t USI::counterValue()
{
    return USISR & 0b1111;
}

void USI::setCounterValue(uint8_t val)
{
    USISR = (USISR & ~0b00001111) | (val & 0b1111);
}

//control regs:
void USI::enableStartInt()
{
    USICR |= _BV(USISIE);
}

void USI::disableStartInt()
{
    USICR &= ~_BV(USISIE);
}

void USI::enableOvfInt()
{
    USICR |= _BV(USIOIE);
}

void USI::disableOvfInt()
{
    USICR &= ~_BV(USIOIE);
}

void USI::setWireMode(WireMode mode)
{
    USICR = ((USICR & ~0b110000) | (((uint8_t)mode) << 4)) ;
}

WireMode USI::getWireMode()
{
    return (WireMode)((USICR >> 4) & 0b11);
}

void USI::setClockMode(ClockMode mode)
{
    USICR = (USICR & ~0b1110) | ((((uint8_t)mode) << 1) & 0b1110);
}

ClockMode USI::getClockMode()
{
    return (ClockMode)((USICR >> 1) & 0b111);
}

void USI::shiftClockStrobeCounter()
{
    USICR |= _BV(USICLK);
}

void USI::toggleClockPortPin()
{
    USICR |= _BV(USITC);
}

//line regs:
void USI::enableSDAOpenDrain()
{
    DDR_USI |= _BV(PORT_USI_SDA);
}

void USI::disableSDAOpenDrain()
{
    DDR_USI &= ~ _BV(PORT_USI_SDA);
}

bool USI::getSDAState()
{
    return PIN_USI & (1 << PIN_USI_SDA);
}

void USI::enableForceHoldSDA()
{
    PORT_USI &= ~ _BV(PORT_USI_SDA);
}

void USI::disableForceHoldSDA()
{
    PORT_USI |= _BV(PORT_USI_SDA);
}

void USI::enableSCLOpenDrain()
{
    DDR_USI |= _BV(PORT_USI_SCL);
}

void USI::disableSCLOpenDrain()
{
    DDR_USI &= ~ _BV(PORT_USI_SCL);
}

bool USI::getSCLState()
{
    return PIN_USI & (1 << PIN_USI_SCL);
}

void USI::enableForceHoldSCL()
{
    PORT_USI &= ~ _BV(PORT_USI_SCL);
}

void USI::disableForceHoldSCL()
{
    PORT_USI |= _BV(PORT_USI_SCL);
}

USI::USI() : data(USIDR), buffer(USIBR) {}

ISR(USI_START_VECTOR)
{
    USI::instance()->isrStrt();
}

ISR(USI_OVERFLOW_VECTOR)
{
    USI::instance()->isrOvfl();
}
