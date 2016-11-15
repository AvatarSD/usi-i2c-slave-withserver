#include "usi.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include "pindefs.h"

//data regs:
volatile uint8_t & USI::data = USIDR;
volatile uint8_t & USI::buffer = USIBR;

//status regs:
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
    cli();
    //    USISR &= ~ 0b1111;
    //    USISR |= val & 0b1111;

    //    for(char i = 0; i < 4; i++)
    //        if((val >> i) & 0b1) USISR |= _BV(i);
    //        else USISR &= ~ _BV(i);

    USISR = (USISR & ~0b00001111) | (val & 0b1111);
    sei();
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

void USI::setWireMode(USI::WireMode mode)
{
    cli();
    USICR = (USICR & ~0b110000) | ((((uint8_t)mode) << 4) & 0b110000);
    sei();
}

USI::WireMode USI::getWireMode()
{
    return (USI::WireMode)((USICR >> 4) & 0b11);
}

void USI::setClockMode(USI::ClockMode mode)
{
    cli();
    USICR = (USICR & ~0b1110) | ((((uint8_t)mode) << 1) & 0b1110);
    sei();
}

USI::ClockMode USI::getClockMode()
{
    return (USI::ClockMode)((USICR >> 1) & 0b111);
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
void USI::holdDataLine()
{
    DDR_USI |= _BV(PORT_USI_SDA);
    //PORT_USI &=~ _BV(PORT_USI_SDA);
}

void USI::releaseDataLine()
{
    DDR_USI &= ~ _BV(PORT_USI_SDA);
    //PORT_USI &=~ _BV(PORT_USI_SDA);
}

void USI::holdClockLine()
{
    DDR_USI |= _BV(PORT_USI_SCL);
    //PORT_USI &= ~ _BV(PORT_USI_SCL);
}

void USI::releaseClockLine()
{
    DDR_USI &= ~ _BV(PORT_USI_SDA);
    //PORT_USI &= ~ _BV(PORT_USI_SDA);
}
