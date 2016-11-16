/**
 * Example sketch for writing to and reading from a slave in transactional
 * manner
 *
 * NOTE: You must not use delay() or I2C communications will fail, use
 * tws_delay() instead (or preferably some smarter timing system)
 *
 * On write the first byte received is considered the register addres to
 * modify/read
 * On each byte sent or read the register address is incremented (and it will
 * loop back to 0)
 *
 * You can try this with the Arduino I2C REPL sketch at
 * https://github.com/rambo/I2C/blob/master/examples/i2crepl/i2crepl.ino
 * If you have bus-pirate remember that the older revisions do not like the
 * slave streching the clock, this leads to all sorts of weird behaviour
 *
 * To read third value (register number 2 since counting starts at 0) send "[ 8
 * 2 [ 9 r ]", value read should be 0xBE
 * If you then send "[ 9 r r r ]" you should get 0xEF 0xDE 0xAD as response
 * (demonstrating the register counter looping back to zero)
 *
 * You need to have at least 8MHz clock on the ATTiny for this to work (and in
 * fact I have so far tested it only on ATTiny85 @8MHz using internal
 * oscillator)
 * Remember to "Burn bootloader" to make sure your chip is in correct mode
 */

/**
 * Pin notes by Suovula, see also http://hlt.media.mit.edu/?p=1229
 *
 * DIP and SOIC have same pinout, however the SOIC chips are much cheaper,
especially if you buy more than 5 at a time
 * For nice breakout boards see https://github.com/rambo/attiny_boards
 *
 * Basically the arduino pin numbers map directly to the PORTB bit numbers.
 *

 */

#include "server.h"
#include <usiTwiSlave.h>

#define ADDRESS_ERR -1

uint8_t I2C_SLAVE_ADDRESS = 0x4;

volatile uint8_t i2c_regs[] = {
    "When switching between tri-state ({DDxn, PORTxn} = 0b00) and output high "
    "({DDxn, PORTxn} = 0b11), an intermediate state with either pull-up enabled "
    "{DDxn, PORTxn} = 0b01) or output low ({DDxn, PORTxn} = 0b10) must occur.  SD"
};



// Tracks the current register pointer position
volatile uint16_t reg_position;
const uint16_t reg_size = sizeof(i2c_regs);

UsiTwiSlave * device;

#if TWI_REQUIRE_BUFFERS
/**
 * This is called for each read request we receive, never put more than one byte
 * of data (with TinyWireS.send) to the
 * send-buffer when using this callback
 */
void requestEvent()
{
    device->put(i2c_regs[reg_position++]);
    if(reg_position >= reg_size)
        reg_position = 0;
}
#else
int16_t requestEvent(uint8_t num)
{
    if(reg_position >= reg_size) {
        reg_position = 0;
        return ADDRESS_ERR;
    }
    return i2c_regs[reg_position++];
}
#endif

#if TWI_REQUIRE_BUFFERS
/**
 * The I2C data received -handler
 *
 * This needs to complete before the next incoming transaction (start, data,
 * restart/stop) on the bus does
 * so be quick, set flags for long running tasks to be called from the mainloop
 * instead of running them directly,
 */
void receiveEvent(uint8_t howMany)
{
    reg_position = device->get();
    if(reg_position >= reg_size) return;
    while(--howMany) {
        i2c_regs[reg_position++] = device->get();
        if(reg_position >= reg_size)
            reg_position = 0;
    }
}
#else
int8_t receiveEvent(uint8_t num, uint8_t data)
{
    if(num == 0) {
        if(data < reg_size)
            reg_position = data;
        else
            return -1;
        return 0;
    }
    //    uint16_t pos = (uint16_t)reg_position + num-1;
    //    if(pos >= reg_size) {
    //        reg_position = 0;
    //        return -1;
    //    }

    if(reg_position >= reg_size) {
        reg_position = 0;
        return ADDRESS_ERR;
    }
    i2c_regs[reg_position++] = data;
    return 0;
}
#endif


void setup()
{
    device = UsiTwiSlave::getInstance();
    device->init(I2C_SLAVE_ADDRESS);
    device->onReceiveSetHandler(&receiveEvent);
    device->onRequestSetHandler(&requestEvent);
}

void loop()
{
    /**
     * This is the only way we can detect stop condition
     * (http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&p=984716&sid=82e9dc7299a8243b86cf7969dd41b5b5#984716)
     * it needs to be called in a very tight loop in order not to miss any
     * (REMINDER: Do *not* use delay() anywhere, use tws_delay() instead).
     * It will call the function registered via TinyWireS.onReceive(); if there is
     * data in the buffer on stop.
     */
#if TWI_REQUIRE_STOP_CHEK
    device->checkStopAndReceiveCall();
#endif
}
