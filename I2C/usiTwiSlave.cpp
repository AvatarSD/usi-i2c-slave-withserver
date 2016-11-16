#include "usi.h"
#include "usiTwiSlave.h"

#define ERROR -1

UsiTwiSlave::UsiTwiSlave()
{
    rxTail = 0;
    rxHead = 0;
    rxCount = 0;
    txTail = 0;
    txHead = 0;
    txCount = 0;

    slaveAddress = 0;

    USI::overflowHandler = &overflowVec;
    USI::startConditionHandler = &startConditionVec;
}

UsiTwiSlave * UsiTwiSlave::getInstance()
{
    static UsiTwiSlave instance;
    return &instance;
}

void UsiTwiSlave::init(uint8_t address)
{
    slaveAddress = address;

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

    /* Shift Register Clock Source = External, positive edge */
    /* 4-Bit Counter: Source = external,both edges */
    USI::setClockMode(ClockMode::EXT_POS);
    /* clear counter, all interrupt flags, except Start Cond */
    USI::setStatus(1, 1, 1, 1, 0x00);
    /* disable Overflow Interrupt */
    USI::disableOvfInt();
    /* enable Start Condition Interrupt */
    USI::enableStartInt();
}


void UsiTwiSlave::SET_USI_TO_TWI_START_CONDITION_MODE()
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

void UsiTwiSlave::SET_USI_TO_SEND_ACK()
{
    /* prepare ACK */
    USI::data = 0;
    /* set SDA as output */
    USI::enableSDAOpenDrain();
    /* set USI counter to shift 1 bit */
    /* clear all interrupt flags, except Start Cond */
    USI::setStatus(0, 1, 1, 1, 0x0E);
}

void UsiTwiSlave::SET_USI_TO_READ_ACK()
{
    /* set SDA as input */
    USI::disableSDAOpenDrain();
    /* prepare ACK */
    USI::data = 0; //USIDR = 0;
    /* set USI counter to shift 1 bit */
    /* clear all interrupt flags, except Start Cond */
    USI::setStatus(0, 1, 1, 1, 0x0E);
}

void UsiTwiSlave::SET_USI_TO_SEND_DATA()
{
    /* set SDA as output */
    USI::enableSDAOpenDrain();
    /* set USI counter to shift 1 bit */
    /* clear all interrupt flags, except Start Cond */
    USI::setStatus(0, 1, 1, 1, 0x00);
}

void UsiTwiSlave::SET_USI_TO_READ_DATA()
{
    /* set SDA as input */
    USI::disableSDAOpenDrain();
    /* set USI counter to shift 1 bit */
    /* clear all interrupt flags, except Start Cond */
    USI::setStatus(0, 1, 1, 1, 0x00);
}


void UsiTwiSlave::startConditionHandler()
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
    overflowState = CHECK_ADDRESS;

    USI::disableSDAOpenDrain();

    // wait for SCL to go low to ensure the Start Condition has completed (the
    // start detector will hold SCL low ) - if a Stop Condition arises then leave
    // the interrupt to prevent waiting forever - don't use USISR to test for Stop
    // Condition as in Application Note AVR312 because the Stop Condition Flag is
    // going to be set from the last TWI sequence
    while(USI::getSCLState() && !USI::getSDAState());

    if(!USI::getSDAState()) {
        // a Stop Condition did not occur
        USI::enableOvfInt();
        USI::setWireMode(WireMode::TWI_WAIT);
    } else {
        // a Stop Condition did occur
        USI::disableOvfInt();
        USI::setWireMode(WireMode::TWI);
    }

    USI::setStatus(1, 1, 1, 1, 0x00);
}

void UsiTwiSlave::overflowHandler()
{
    uint8_t dataRegBuff = USI::data;
    switch(overflowState) {

    // Address mode: check address and send ACK, else reset USI
    case CHECK_ADDRESS:
        if((dataRegBuff == 0) || ((dataRegBuff >> 1) == slaveAddress)) {
            if(dataRegBuff & 0x01) {
                requestCall();
                overflowState = SEND_DATA;  // master want reading - transmitting
            } else
                overflowState = RECEIVE_DATA; // master want writing - receiving
            SET_USI_TO_SEND_ACK();
        } else
            SET_USI_TO_TWI_START_CONDITION_MODE();
        break;

    // Master write data mode: check reply and goto USI_SLAVE_SEND_DATA if OK,
    // else reset USI
    case CHECK_REPLY_FROM_ACK:
        if(dataRegBuff) { // if NACK, the master does not want more data
            SET_USI_TO_TWI_START_CONDITION_MODE();
            return;
        }

    // from here we just drop straight into USI_SLAVE_SEND_DATA if the
    // master sent an ACK
    // copy data from buffer to USIDR and set USI to shift byte
    case SEND_DATA:
        if(txWaitCount())
            USI::data = getFromTXBuff();
        else {
            SET_USI_TO_TWI_START_CONDITION_MODE();
            return;
        }
        overflowState = CHECK_ACK_FROM_SEND_DATA;
        SET_USI_TO_SEND_DATA();
        break;

    // set USI to sample reply from master
    case CHECK_ACK_FROM_SEND_DATA:
        overflowState = CHECK_REPLY_FROM_ACK;
        SET_USI_TO_READ_ACK();
        break;

    // Master read data mode: set USI to sample data from master
    case RECEIVE_DATA:
        overflowState = GET_DATA_AND_SEND_ACK;
        SET_USI_TO_READ_DATA();
        break;

    // copy data from USIDR and send ACK
    case GET_DATA_AND_SEND_ACK:
        overflowState = RECEIVE_DATA;
        putIntoRXBuff(dataRegBuff);
        SET_USI_TO_SEND_ACK();
        break;
    } // end switch
}



int16_t UsiTwiSlave::requestCall()
{
    receiveCall();

    if(onRequest)
        return onRequest();
    return ERROR;
}

void UsiTwiSlave::receiveCall()
{
    if(onReceiver)
        if(available())
            onReceiver(available());
}

void UsiTwiSlave::checkStopAndReceiveCall()
{
    if(USI::getStopCondIntFlag())
        receiveCall();
}

void UsiTwiSlave::onReceiveSetHandler(void (*func)(uint8_t))
{
    onReceiver = func;
}

void UsiTwiSlave::onRequestSetHandler(int16_t (*func)())
{
    onRequest = func;
}


void UsiTwiSlave::put(uint8_t data)
{
    // wait for free space in buffer
    while(txCount == TWI_TX_BUFFER_SIZE);

    txBuf[txHead] = data;
    if(++txHead == TWI_TX_BUFFER_SIZE)
        txHead = 0;
    txCount++;
}

uint8_t UsiTwiSlave::getFromTXBuff()
{
    if(txCount == 0) return 0;

    uint8_t data = txBuf[txTail];
    if(++txTail == TWI_TX_BUFFER_SIZE)
        txTail = 0;
    txCount--;

    return data;
}

uint8_t UsiTwiSlave::get(void)
{
    // wait for Rx data
    while(!rxCount);

    uint8_t rtn_byte = rxBuf[rxTail];
    if(++rxTail == TWI_RX_BUFFER_SIZE)
        rxTail = 0;
    rxCount--;

    return rtn_byte;
}

void UsiTwiSlave::putIntoRXBuff(uint8_t data)
{
    // check buffer size
    if(rxCount == TWI_RX_BUFFER_SIZE) return;

    rxBuf[rxHead] = data;
    if(++rxHead == TWI_RX_BUFFER_SIZE)
        rxHead = 0;
    rxCount++;
}

uint8_t UsiTwiSlave::available(void)
{
    return rxCount;
}

uint8_t UsiTwiSlave::txWaitCount(void)
{
    return txCount;
}


void UsiTwiSlave::startConditionVec()
{
    getInstance()->startConditionHandler();
}

void UsiTwiSlave::overflowVec()
{
    getInstance()->overflowHandler();
}
