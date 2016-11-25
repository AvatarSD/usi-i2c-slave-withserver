#include "usi.h"
#include "usiTwiSlave.h"


UsiTwiSlave::UsiTwiSlave()
{
    startCounter = 0;
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
    SET_USI_TO_TWI_START_CONDITION_MODE();
    // set SCL high
    USI::disableForceHoldSCL();
    // set SDA high
    USI::disableForceHoldSDA();
    // Set SDA as input
    USI::enableSCLOpenDrain();
}

uint8_t UsiTwiSlave::getAddress()
{
    return slaveAddress;
}


void UsiTwiSlave::SET_USI_TO_TWI_START_CONDITION_MODE()
{
    /* set USI counter to shift 1 bit */
    /* clear all interrupt flags, except Start Cond */
    USI::setStatus(1, 1, 1, 1, 0x00);
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
    // set default starting conditions for new TWI package
    overflowState = CHECK_ADDRESS;
    USI::disableSDAOpenDrain();
    startCounter = 0;

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
    int16_t tmp;

    switch(overflowState) {

    /***************************************************************************************/

    // Address mode: check address and send ACK, else reset USI
    case CHECK_ADDRESS: // TODO auto assigment adderess
        if((dataRegBuff >> 1) == slaveAddress) {
            if(dataRegBuff & 0x01) {
                overflowState = SEND_DATA;  // master want reading - transmitting
            } else
                overflowState = RECEIVE_DATA; // master want writing - receiving
            SET_USI_TO_SEND_ACK();
        } else
            SET_USI_TO_TWI_START_CONDITION_MODE();
        break;

    /***************************************************************************************/

    // from here we just drop straight into USI_SLAVE_SEND_DATA if the
    // master sent an ACK
    // copy data from buffer to USIDR and set USI to shift byte
    case SEND_DATA:
        tmp = requestCall(startCounter++);
        if(tmp >= 0)
            USI::data = tmp;
        else {
            SET_USI_TO_TWI_START_CONDITION_MODE();
            return;
        }
        overflowState = CHECK_ACK_FROM_SEND_DATA;
        SET_USI_TO_SEND_DATA();
        break;

    /***************************************************************************************/

    // set USI to sample reply from master
    case CHECK_ACK_FROM_SEND_DATA:
        overflowState = CHECK_REPLY_FROM_ACK;
        SET_USI_TO_READ_ACK();
        break;

    /***************************************************************************************/

    // Master write data mode: check reply and goto USI_SLAVE_SEND_DATA if OK,
    // else reset USI
    case CHECK_REPLY_FROM_ACK:
        if(dataRegBuff) { // if NACK, the master does not want more data
            SET_USI_TO_TWI_START_CONDITION_MODE();
            return;
        }

    /***************************************************************************************/

    // Master read data mode: set USI to sample data from master
    case RECEIVE_DATA:
        overflowState = GET_DATA_AND_SEND_ACK;
        SET_USI_TO_READ_DATA();
        break;

    /***************************************************************************************/

    // copy data from USIDR and send ACK
    case GET_DATA_AND_SEND_ACK:
        if(receiveCall(startCounter++, dataRegBuff) < 0) {
            SET_USI_TO_TWI_START_CONDITION_MODE();
            return;
        }
        overflowState = RECEIVE_DATA;
        SET_USI_TO_SEND_ACK();
        break;

        /***************************************************************************************/
    }
}


int8_t UsiTwiSlave::receiveCall(uint8_t num, uint8_t data)
{
    if(onReceiver)
        return onReceiver(num, data);
    return ERR;
}

int16_t UsiTwiSlave::requestCall(uint8_t num)
{
    if(onRequest)
        return onRequest(num);
    return ERR;
}

void UsiTwiSlave::onReceiveSetHandler(ReceiveHandler func)
{
    onReceiver = func;
}

void UsiTwiSlave::onRequestSetHandler(RequestHandler func)
{
    onRequest = func;
}

void UsiTwiSlave::startConditionVec()
{
    getInstance()->startConditionHandler();
}

void UsiTwiSlave::overflowVec()
{
    getInstance()->overflowHandler();
}
