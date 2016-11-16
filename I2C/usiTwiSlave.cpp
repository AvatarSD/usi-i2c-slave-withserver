#include "usi.h"
#include "usiTwiSlave.h"


/********************************************************************************

                                local functions

********************************************************************************/

enum overflowState_t {
    USI_SLAVE_CHECK_ADDRESS = 0x00,
    USI_SLAVE_SEND_DATA = 0x01,
    USI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA = 0x02,
    USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA = 0x03,
    USI_SLAVE_REQUEST_DATA = 0x04,
    USI_SLAVE_GET_DATA_AND_SEND_ACK = 0x05
};

uint8_t slaveAddress;
volatile overflowState_t overflowState;

uint8_t rxBuf[TWI_RX_BUFFER_SIZE];
volatile uint8_t rxHead;
volatile uint8_t rxTail;
volatile uint8_t rxCount;

uint8_t txBuf[TWI_TX_BUFFER_SIZE];
volatile uint8_t txHead;
volatile uint8_t txTail;
volatile uint8_t txCount;

// on_XXX handler pointers
void (*usi_onRequestPtr)(void);
void (*usi_onReceiverPtr)(uint8_t);
void (*_onTwiDataRequest)(void);


void SET_USI_TO_TWI_START_CONDITION_MODE()
{
    /* set SDA as input */
    USI::disableSDAOpenDrain();
    /* enable Start Condition Interrupt, disable Overflow Interrupt */
    USI::enableStartInt();
    USI::disableOvfInt();
    /* set USI in Two-wire mode, no USI Counter overflow hold */
    USI::setWireMode(WireMode::TWI);
    /* Shift Register Clock Source = External, positive edge */
    /* 4-Bit Counter: Source = external,both edges */
    USI::setClockMode(ClockMode::EXT_POS);
    /* set USI counter to shift 1 bit */
    /* clear all interrupt flags, except Start Cond */
    USI::setStatus(0, 1, 1, 1, 0x00);
}

void SET_USI_TO_SEND_ACK()
{
    /* prepare ACK */
    USI::data = 0;
    /* set SDA as output */
    USI::enableSDAOpenDrain();
    /* set USI counter to shift 1 bit */
    /* clear all interrupt flags, except Start Cond */
    USI::setStatus(0, 1, 1, 1, 0x0E);
}

void SET_USI_TO_READ_ACK()
{
    /* set SDA as input */
    USI::disableSDAOpenDrain();
    /* prepare ACK */
    USI::data = 0; //USIDR = 0;
    /* set USI counter to shift 1 bit */
    /* clear all interrupt flags, except Start Cond */
    USI::setStatus(0, 1, 1, 1, 0x0E);
}

void SET_USI_TO_SEND_DATA()
{
    /* set SDA as output */
    USI::enableSDAOpenDrain();
    /* set USI counter to shift 1 bit */
    /* clear all interrupt flags, except Start Cond */
    USI::setStatus(0, 1, 1, 1, 0x00);
}

void SET_USI_TO_READ_DATA()
{
    /* set SDA as input */
    USI::disableSDAOpenDrain();
    /* set USI counter to shift 1 bit */
    /* clear all interrupt flags, except Start Cond */
    USI::setStatus(0, 1, 1, 1, 0x00);
}

void USI_RECEIVE_CALLBACK()
{
    if(usi_onReceiverPtr) {
        if(usiTwiAmountDataInReceiveBuffer())
            usi_onReceiverPtr(usiTwiAmountDataInReceiveBuffer());
    }
}

void ONSTOP_USI_RECEIVE_CALLBACK()
{
    if(USI::getStopCondIntFlag())//USISR & (1 << USIPF))
        USI_RECEIVE_CALLBACK();
}

void USI_REQUEST_CALLBACK()
{
    USI_RECEIVE_CALLBACK();

    if(usi_onRequestPtr)
        usi_onRequestPtr();
}


void putIntoRXBuff(uint8_t data)
{
    // check buffer size
    if(rxCount < TWI_RX_BUFFER_SIZE) {
        rxBuf[rxHead] = data;
        rxHead = (rxHead + 1) & TWI_RX_BUFFER_MASK;
        rxCount++;
    } else {
        // overrun
        // drop data
    }
}

uint8_t getFromTXBuff()
{
    uint8_t data = txBuf[txTail];
    txTail = (txTail + 1) & TWI_TX_BUFFER_MASK;
    txCount--;
    return data;
}


void startConditionHandler()
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

    USI::disableSDAOpenDrain();


    // wait for SCL to go low to ensure the Start Condition has completed (the
    // start detector will hold SCL low ) - if a Stop Condition arises then leave
    // the interrupt to prevent waiting forever - don't use USISR to test for Stop
    // Condition as in Application Note AVR312 because the Stop Condition Flag is
    // going to be set from the last TWI sequence
    while(USI::getSCLState() && !USI::getSDAState());

    if(!USI::getSDAState()) {
        // a Stop Condition did not occur
        /*USICR =
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
            (0 << USITC); */

        USI::enableOvfInt();
        USI::setWireMode(WireMode::TWI_WAIT);
    } else {
        // a Stop Condition did occur
        /*USICR =
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
            (0 << USITC);*/

        USI::disableOvfInt();
        USI::setWireMode(WireMode::TWI);
    }

    USI::setStatus(1, 1, 1, 1, 0x00);
}

void overflowHandler()
{
    uint8_t dataRegBuff = USI::data;

    switch(overflowState) {
    // Address mode: check address and send ACK (and next USI_SLAVE_SEND_DATA)
    // if OK,
    // else reset USI
    case USI_SLAVE_CHECK_ADDRESS:
        if((dataRegBuff == 0) || ((dataRegBuff >> 1) == slaveAddress)) {
            if(dataRegBuff & 0x01) {
                USI_REQUEST_CALLBACK();
                overflowState = USI_SLAVE_SEND_DATA;
            } else
                overflowState = USI_SLAVE_REQUEST_DATA; // end if

            SET_USI_TO_SEND_ACK();
        } else
            SET_USI_TO_TWI_START_CONDITION_MODE();

        break;

    // Master write data mode: check reply and goto USI_SLAVE_SEND_DATA if OK,
    // else reset USI
    case USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA:
        if(dataRegBuff) {
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
        if(usiTwiDataInTransmitBuffer())
            USI::data = getFromTXBuff();
        else {
            // the buffer is empty
            //            SET_USI_TO_READ_ACK(); // This might be neccessary sometimes see
            //            // http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&p=805227#805227
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
        putIntoRXBuff(dataRegBuff);
        overflowState = USI_SLAVE_REQUEST_DATA;
        SET_USI_TO_SEND_ACK();
        break;
    } // end switch
}


/********************************************************************************

                                public functions

********************************************************************************/

// initialise USI for TWI slave mode
void usiTwiSlaveInit(uint8_t ownAddress)
{
    slaveAddress = ownAddress;
    rxTail = 0;
    rxHead = 0;
    rxCount = 0;
    txTail = 0;
    txHead = 0;
    txCount = 0;

    USI::overflowHandler = &overflowHandler;
    USI::startConditionHandler = &startConditionHandler;

    /* set USI in Two-wire mode, no USI Counter overflow hold */
    USI::setWireMode(WireMode::TWI);
    // set SCL high
    USI::disableForceHoldSCL();
    // set SDA high
    USI::disableForceHoldSDA();
    // Set SDA as input
    USI::enableSCLOpenDrain();
    // Set SDA as output
    USI::disableSDAOpenDrain();
    /* disable Overflow Interrupt */
    USI::disableOvfInt();
    /* Shift Register Clock Source = External, positive edge */
    /* 4-Bit Counter: Source = external,both edges */
    USI::setClockMode(ClockMode::EXT_POS);
    /* clear counter, all interrupt flags, except Start Cond */
    USI::setStatus(1, 1, 1, 1, 0x00);
    /* enable Start Condition Interrupt */
    USI::enableStartInt();
}

// put data in the transmission buffer, wait if buffer is full
void usiTwiTransmitByte(uint8_t data)
{
    // wait for free space in buffer
    while(txCount == TWI_TX_BUFFER_SIZE);

    // store data in buffer
    txBuf[txHead] = data;
    txHead = (txHead + 1) & TWI_TX_BUFFER_MASK;
    txCount++;
}

// return a byte from the receive buffer, wait if buffer is empty
uint8_t usiTwiReceiveByte(void)
{
    // wait for Rx data
    while(!rxCount);

    uint8_t rtn_byte = rxBuf[rxTail];
    // calculate buffer index
    rxTail = (rxTail + 1) & TWI_RX_BUFFER_MASK;
    rxCount--;
    // return data from the buffer.
    return rtn_byte;
}

uint8_t usiTwiAmountDataInReceiveBuffer(void)
{
    return rxCount;
}

bool usiTwiDataInTransmitBuffer(void)
{
    return txCount;
}
