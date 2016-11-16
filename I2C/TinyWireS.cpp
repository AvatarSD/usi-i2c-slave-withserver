/*
  TinyWireS.cpp - a wrapper class for Don Blake's usiTwiSlave routines.
  Provides TWI/I2C Slave functionality on ATtiny processers in Arduino
  environment.
  1/23/2011 BroHogan -  brohoganx10 at gmail dot com

  **** See TinyWireS.h for Credits and Usage information ****

  This library is free software; you can redistribute it and/or modify it under
  the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2.1 of the License, or any later version.
  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A
  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "TinyWireS.h"
#include "usiTwiSlave.h"
#include <avr/interrupt.h>

#include <inttypes.h>

USI_TWI_S::USI_TWI_S() : device(UsiTwiSlave::getInstance())
{

}

void USI_TWI_S::begin(uint8_t slaveAddr)
{
    device->init(slaveAddr);
}

void USI_TWI_S::send(uint8_t data)
{
    // send it back to master
    device->put(data);
}

uint8_t USI_TWI_S::available()
{
    // the bytes available that haven't been read yet
    return device->available();
    // return usiTwiDataInReceiveBuffer(); // This is wrong as far as the Wire API
    // is concerned since it returns boolean and not amount
}

uint8_t USI_TWI_S::receive()
{
    // returns the bytes received one at a time
    return device->get();
}

// sets function called on slave write
void USI_TWI_S::onReceive(void (*function)(uint8_t))
{
    device->onReceiver = function;
}

// sets function called on slave read
void USI_TWI_S::onRequest(void (*function)(void))
{
    device->onRequest = function;
}

void USI_TWI_S::TinyWireS_stop_check()
{
    if(!device->onReceiver) {
        // no onReceive callback, nothing to do...
        return;
    }

    if(!(USISR & (1 << USIPF))) {
        // Stop not detected
        return;
    }

    uint8_t amount = device->available();

    if(amount == 0) {
        // no data in buffer
        return;
    }

    device->onReceiver(amount);
}
