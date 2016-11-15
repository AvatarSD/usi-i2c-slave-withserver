#include "usi.h"
#include "usiTwiSlave.h"

#include "pindefs.h"
#include <avr/interrupt.h>
#include <avr/io.h>

/********************************************************************************

                                local functions

********************************************************************************/

enum overflowState_t
{
    USI_SLAVE_CHECK_ADDRESS = 0x00,
    USI_SLAVE_SEND_DATA = 0x01,
    USI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA = 0x02,
    USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA = 0x03,
    USI_SLAVE_REQUEST_DATA = 0x04,
    USI_SLAVE_GET_DATA_AND_SEND_ACK = 0x05
};

static uint8_t slaveAddress;
static volatile overflowState_t overflowState;

static uint8_t rxBuf[TWI_RX_BUFFER_SIZE];
static volatile uint8_t rxHead;
static volatile uint8_t rxTail;
static volatile uint8_t rxCount;

static uint8_t txBuf[TWI_TX_BUFFER_SIZE];
static volatile uint8_t txHead;
static volatile uint8_t txTail;
static volatile uint8_t txCount;

// flushes the TWI buffers
static void
flushTwiBuffers(void)
{
    rxTail = 0;
    rxHead = 0;
    rxCount = 0;
    txTail = 0;
    txHead = 0;
    txCount = 0;
}

// on_XXX handler pointers
void (*usi_onRequestPtr)(void);
void (*usi_onReceiverPtr)(uint8_t);
void (*_onTwiDataRequest)(void);

void clearIntFlagExeptStart()
{
    USI::releaseStartCondLock();
    USI::clearOvfIntFlag();
    USI::clearStopCondIntFlag();
    USI::clearCollisionFlag();
}

void
SET_USI_TO_SEND_ACK()
{
    /* prepare ACK */
    USI::data = 0;
    //USIDR = 0;

    /* set SDA as output */
    USI::holdDataLine();
    //DDR_USI |= (1 << PORT_USI_SDA);


    //    /* clear all interrupt flags, except Start Cond */
    //    USISR = (0 << USI_START_COND_INT)
    //            | (1 << USIOIF)
    //            | (1 << USIPF)
    //            | (1 << USIDC)
    //            /* set USI counter to shift 1 bit */
    //            | (0x0E << USICNT0);

    /* set USI counter to shift 1 bit */
    USI::setCounterValue(0x0E);
    /* clear all interrupt flags, except Start Cond */
    clearIntFlagExeptStart();
}

void
SET_USI_TO_READ_ACK()
{
    /* set SDA as input */
    USI::releaseDataLine();
    //DDR_USI &= ~(1 << PORT_USI_SDA);

    /* prepare ACK */
    USI::data = 0; //USIDR = 0;


    //    /* clear all interrupt flags, except Start Cond */
    //    USISR = (0 << USI_START_COND_INT) | (1 << USIOIF) | (1 << USIPF) |
    //            (1 << USIDC) |
    //            /* set USI counter to shift 1 bit */
    //            (0x0E << USICNT0);

    /* set USI counter to shift 1 bit */
    USI::setCounterValue(0x0E);
    /* clear all interrupt flags, except Start Cond */
    clearIntFlagExeptStart();
}

void
SET_USI_TO_TWI_START_CONDITION_MODE()
{
    /* enable Start Condition Interrupt, disable Overflow Interrupt */
    USI::enableStartInt();
    USI::disableOvfInt();
    /* set USI in Two-wire mode, no USI Counter overflow hold */
    USI::setWireMode(USI::WireMode::TWI);
    /* Shift Register Clock Source = External, positive edge */
    /* 4-Bit Counter: Source = external,both edges */
    USI::setClockMode(USI::EXT_POS);


    //    USISR = /* clear all interrupt flags, except Start Cond */
    //        (0 << USI_START_COND_INT) | (1 << USIOIF) | (1 << USIPF) | (1 << USIDC) |
    //        (0x0 << USICNT0);

    /* set USI to shift out 8 bits */
    USI::setCounterValue(0x00);
    /* clear all interrupt flags, except Start Cond */
    clearIntFlagExeptStart();
}

void
SET_USI_TO_SEND_DATA()
{
    //    /* set SDA as output */
    //    DDR_USI |= (1 << PORT_USI_SDA);
    USI::holdDataLine();

    /* clear all interrupt flags, except Start Cond */
    //    USISR = (0 << USI_START_COND_INT) | (1 << USIOIF) | (1 << USIPF) |
    //            (1 << USIDC) | /* set USI to shift out 8 bits */
    //            (0x0 << USICNT0);

    /* set USI to shift out 8 bits */
    USI::setCounterValue(0x00);
    /* clear all interrupt flags, except Start Cond */
    clearIntFlagExeptStart();
}

void
SET_USI_TO_READ_DATA()
{
    /* set SDA as input */
    //    DDR_USI &= ~(1 << PORT_USI_SDA);
    USI::releaseDataLine();

    //    /* clear all interrupt flags, except Start Cond */
    //    USISR = (0 << USI_START_COND_INT) | (1 << USIOIF) | (1 << USIPF) |
    //            (1 << USIDC) | /* set USI to shift out 8 bits */
    //            (0x0 << USICNT0);


    /* set USI to shift out 8 bits */
    USI::setCounterValue(0x00);
    /* clear all interrupt flags, except Start Cond */
    clearIntFlagExeptStart();

    //    cli();
    //    USISR &= ~ _BV(0);
    //    USISR &= ~ _BV(1);
    //    USISR &= ~ _BV(2);
    //    USISR &= ~ _BV(3);
    /*    USISR &= ~(1 << USI_START_COND_INT);
        USISR |= (1 << USIOIF) |
                 (1 << USIPF) |
                 (1 << USIDC);*/ /* set USI to shift out 8 bits */
    //    //    USISR &= ~(0x1111 << USICNT0);

    //    sei();

}

void
USI_RECEIVE_CALLBACK()
{
    if(usi_onReceiverPtr)
    {
        if(usiTwiAmountDataInReceiveBuffer())
            usi_onReceiverPtr(usiTwiAmountDataInReceiveBuffer());
    }
}

void
ONSTOP_USI_RECEIVE_CALLBACK()
{
    if(USISR & (1 << USIPF))
        USI_RECEIVE_CALLBACK();
}

void
USI_REQUEST_CALLBACK()
{
    USI_RECEIVE_CALLBACK();

    if(usi_onRequestPtr)
        usi_onRequestPtr();
}

ISR(USI_START_VECTOR)
{
    /*
    // This triggers on second write, but claims to the callback there is only
    *one* byte in buffer
    ONSTOP_USI_RECEIVE_CALLBACK();
    */
    /*
    // This triggers on second write, but claims to the callback there is only
    *one* byte in buffer
    USI_RECEIVE_CALLBACK();
    */
    // set default starting conditions for new TWI package
    overflowState = USI_SLAVE_CHECK_ADDRESS;
    // set SDA as input
    DDR_USI &= ~(1 << PORT_USI_SDA);

    // wait for SCL to go low to ensure the Start Condition has completed (the
    // start detector will hold SCL low ) - if a Stop Condition arises then leave
    // the interrupt to prevent waiting forever - don't use USISR to test for Stop
    // Condition as in Application Note AVR312 because the Stop Condition Flag is
    // going to be set from the last TWI sequence
    while(
        // SCL his high
        (PIN_USI & (1 << PIN_USI_SCL)) &&
        // and SDA is low
        !((PIN_USI & (1 << PIN_USI_SDA))))
        ;

    if(!(PIN_USI & (1 << PIN_USI_SDA)))
    {
        // a Stop Condition did not occur
        USICR =
            // keep Start Condition Interrupt enabled to detect RESTART
            (1 << USISIE) |
            // enable Overflow Interrupt
            (1 << USIOIE) |
            // set USI in Two-wire mode, hold SCL low on USI Counter overflow
            (1 << USIWM1) | (1 << USIWM0) |
            // Shift Register Clock Source = External, positive edge
            // 4-Bit Counter Source = external, both edges
            (1 << USICS1) | (0 << USICS0) | (0 << USICLK) |
            // no toggle clock-port pin
            (0 << USITC);
    }
    else
    {
        // a Stop Condition did occur
        USICR =
            // enable Start Condition Interrupt
            (1 << USISIE) |
            // disable Overflow Interrupt
            (0 << USIOIE) |
            // set USI in Two-wire mode, no USI Counter overflow hold
            (1 << USIWM1) | (0 << USIWM0) |
            // Shift Register Clock Source = external, positive edge
            // 4-Bit Counter Source = external, both edges
            (1 << USICS1) | (0 << USICS0) | (0 << USICLK) |
            // no toggle clock-port pin
            (0 << USITC);
    } // end if

    USISR =
        // clear interrupt flags - resetting the Start Condition Flag will
        // release SCL
        (1 << USI_START_COND_INT) | (1 << USIOIF) | (1 << USIPF) | (1 << USIDC) |
        // set USI to sample 8 bits (count 16 external SCL pin toggles)
        (0x0 << USICNT0);
}

ISR(USI_OVERFLOW_VECTOR)
{
    switch(overflowState)
    {
    // Address mode: check address and send ACK (and next USI_SLAVE_SEND_DATA)
    // if OK,
    // else reset USI
    case USI_SLAVE_CHECK_ADDRESS:
        if((USIDR == 0) || ((USIDR >> 1) == slaveAddress))
        {
            if(USIDR & 0x01)
            {
                USI_REQUEST_CALLBACK();
                overflowState = USI_SLAVE_SEND_DATA;
            }
            else
                overflowState = USI_SLAVE_REQUEST_DATA; // end if

            SET_USI_TO_SEND_ACK();
        }
        else
            SET_USI_TO_TWI_START_CONDITION_MODE();

        break;

    // Master write data mode: check reply and goto USI_SLAVE_SEND_DATA if OK,
    // else reset USI
    case USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA:
        if(USIDR)
        {
            // if NACK, the master does not want more data
            SET_USI_TO_TWI_START_CONDITION_MODE();
            return;
        }

    // from here we just drop straight into USI_SLAVE_SEND_DATA if the
    // master sent an ACK
    // copy data from buffer to USIDR and set USI to shift byte
    // next USI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA
    case USI_SLAVE_SEND_DATA:

        // Get data from Buffer
        if(txCount)
        {
            USIDR = txBuf[txTail];
            txTail = (txTail + 1) & TWI_TX_BUFFER_MASK;
            txCount--;
        }
        else
        {
            // the buffer is empty
            SET_USI_TO_READ_ACK(); // This might be neccessary sometimes see
            // http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&p=805227#805227
            SET_USI_TO_TWI_START_CONDITION_MODE();
            return;
        } // end if

        overflowState = USI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA;
        SET_USI_TO_SEND_DATA();
        break;

    // set USI to sample reply from master
    // next USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA
    case USI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA:
        overflowState = USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA;
        SET_USI_TO_READ_ACK();
        break;

    // Master read data mode: set USI to sample data from master, next
    // USI_SLAVE_GET_DATA_AND_SEND_ACK
    case USI_SLAVE_REQUEST_DATA:
        overflowState = USI_SLAVE_GET_DATA_AND_SEND_ACK;
        SET_USI_TO_READ_DATA();
        break;

    // copy data from USIDR and send ACK
    // next USI_SLAVE_REQUEST_DATA
    case USI_SLAVE_GET_DATA_AND_SEND_ACK:

        // put data into buffer
        // check buffer size
        if(rxCount < TWI_RX_BUFFER_SIZE)
        {
            rxBuf[rxHead] = USIDR;
            rxHead = (rxHead + 1) & TWI_RX_BUFFER_MASK;
            rxCount++;
        }
        else
        {
            // overrun
            // drop data
        }

        // next USI_SLAVE_REQUEST_DATA
        overflowState = USI_SLAVE_REQUEST_DATA;
        SET_USI_TO_SEND_ACK();
        break;
    } // end switch
}

/********************************************************************************

                                public functions

********************************************************************************/

// initialise USI for TWI slave mode
void
usiTwiSlaveInit(uint8_t ownAddress)
{
    flushTwiBuffers();
    slaveAddress = ownAddress;
    // In Two Wire mode (USIWM1, USIWM0 = 1X), the slave USI will pull SCL
    // low when a start condition is detected or a counter overflow (only
    // for USIWM1, USIWM0 = 11).  This inserts a wait state.  SCL is released
    // by the ISRs (USI_START_vect and USI_OVERFLOW_vect).
    // Set SCL and SDA as output
    DDR_USI |= (1 << PORT_USI_SCL) | (1 << PORT_USI_SDA);
    // set SCL high
    PORT_USI |= (1 << PORT_USI_SCL);
    // set SDA high
    PORT_USI |= (1 << PORT_USI_SDA);
    // Set SDA as input
    DDR_USI &= ~(1 << PORT_USI_SDA);
    USICR =
        // enable Start Conditionrupt
        (1 << USISIE) |
        // disable Overflow Interrupt
        (0 << USIOIE) |
        // set USI in Two-wire mode, no USI Counter overflow hold
        (1 << USIWM1) | (0 << USIWM0) |
        // Shift Register Clock Source = external, positive edge
        // 4-Bit Counter Source = external, both edges
        (1 << USICS1) | (0 << USICS0) | (0 << USICLK) |
        // no toggle clock-port pin
        (0 << USITC);
    // clear all interrupt flags and reset overflow counter
    USISR =
        (1 << USI_START_COND_INT) | (1 << USIOIF) | (1 << USIPF) | (1 << USIDC);
}

// put data in the transmission buffer, wait if buffer is full
void
usiTwiTransmitByte(uint8_t data)
{
    // uint8_t tmphead;

    // wait for free space in buffer
    while(txCount == TWI_TX_BUFFER_SIZE)
        ;

    // store data in buffer
    txBuf[txHead] = data;
    txHead = (txHead + 1) & TWI_TX_BUFFER_MASK;
    txCount++;
}

// return a byte from the receive buffer, wait if buffer is empty
uint8_t
usiTwiReceiveByte(void)
{
    uint8_t rtn_byte;

    // wait for Rx data
    while(!rxCount)
        ;

    rtn_byte = rxBuf[rxTail];
    // calculate buffer index
    rxTail = (rxTail + 1) & TWI_RX_BUFFER_MASK;
    rxCount--;
    // return data from the buffer.
    return rtn_byte;
}

uint8_t
usiTwiAmountDataInReceiveBuffer(void)
{
    return rxCount;
}

bool
usiTwiDataInTransmitBuffer(void)
{
    return txCount;
}
