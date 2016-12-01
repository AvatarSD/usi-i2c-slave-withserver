#include "usi.h"
#include "usiTwiSlave.h"


UsiTwiSlave::UsiTwiSlave(USI * usi) : usi(usi)
{
    startCounter = 0;
    slaveAddress = 0;
    multicastAddress = 0;
    this->usi->setIsrHandler(this);
}

void UsiTwiSlave::init(iServer * server, uint8_t address,
                       uint8_t multicastAddress)
{
    onEventHandler(server);
    setAddress(address);
    setMulticastAddress(multicastAddress);

    SET_USI_TO_TWI_START_CONDITION_MODE();
    // set SCL high
    usi->disableForceHoldSCL();
    // set SDA high
    usi->disableForceHoldSDA();
    // Set SDA as input
    usi->enableSCLOpenDrain();
}

void UsiTwiSlave::onEventHandler(iServer * server)
{
    this->server = server;
}

uint8_t UsiTwiSlave::getAddress()
{
    return slaveAddress;
}

void UsiTwiSlave::setAddress(uint8_t addr)
{
    slaveAddress = addr;
}

uint8_t UsiTwiSlave::getMulticastAddress()
{
    return multicastAddress;
}

void UsiTwiSlave::setMulticastAddress(uint8_t addr)
{
    multicastAddress = addr;
}


void UsiTwiSlave::SET_USI_TO_TWI_START_CONDITION_MODE()
{
    /* set USI counter to shift 1 bit */
    /* clear all interrupt flags, except Start Cond */
    usi->setStatus(1, 1, 1, 1, 0x00);
    /* set SDA as input */
    usi->disableSDAOpenDrain();
    /* enable Start Condition Interrupt, disable Overflow Interrupt */
    usi->enableStartInt();
    usi->disableOvfInt();
    /* set USI in Two-wire mode, no USI Counter overflow hold */
    usi->setWireMode(WireMode::TWI);
    /* Shift Register Clock Source = External, positive edge */
    /* 4-Bit Counter: Source = external,both edges */
    usi->setClockMode(ClockMode::EXT_POS);
}

void UsiTwiSlave::SET_USI_TO_SEND_ACK()
{
    /* prepare ACK */
    usi->data = 0;
    /* set SDA as output */
    usi->enableSDAOpenDrain();
    /* set USI counter to shift 1 bit */
    /* clear all interrupt flags, except Start Cond */
    usi->setStatus(0, 1, 1, 1, 0x0E);
}

void UsiTwiSlave::SET_USI_TO_READ_ACK()
{
    /* set SDA as input */
    usi->disableSDAOpenDrain();
    /* prepare ACK */
    usi->data = 0; //USIDR = 0;
    /* set USI counter to shift 1 bit */
    /* clear all interrupt flags, except Start Cond */
    usi->setStatus(0, 1, 1, 1, 0x0E);
}

void UsiTwiSlave::SET_USI_TO_SEND_DATA()
{
    /* set SDA as output */
    usi->enableSDAOpenDrain();
    /* set USI counter to shift 1 bit */
    /* clear all interrupt flags, except Start Cond */
    usi->setStatus(0, 1, 1, 1, 0x00);
}

void UsiTwiSlave::SET_USI_TO_READ_DATA()
{
    /* set SDA as input */
    usi->disableSDAOpenDrain();
    /* set USI counter to shift 1 bit */
    /* clear all interrupt flags, except Start Cond */
    usi->setStatus(0, 1, 1, 1, 0x00);
}


void UsiTwiSlave::startConditionHandler()
{
    // set default starting conditions for new TWI package
    overflowState = CHECK_ADDRESS;
    usi->disableSDAOpenDrain();
    startCounter = 0;

    // wait for SCL to go low to ensure the Start Condition has completed (the
    // start detector will hold SCL low ) - if a Stop Condition arises then leave
    // the interrupt to prevent waiting forever - don't use USISR to test for Stop
    // Condition as in Application Note AVR312 because the Stop Condition Flag is
    // going to be set from the last TWI sequence
    while(usi->getSCLState() && !usi->getSDAState());

    if(!usi->getSDAState()) {
        // a Stop Condition did not occur
        usi->enableOvfInt();
        usi->setWireMode(WireMode::TWI_WAIT);
    } else {
        // a Stop Condition did occur
        usi->disableOvfInt();
        usi->setWireMode(WireMode::TWI);
    }

    usi->setStatus(1, 1, 1, 1, 0x00);
}

void UsiTwiSlave::overflowHandler()
{
    uint8_t dataRegBuff = usi->data;

    switch(overflowState) {

    /***************************************************************************************/

    // Address mode: check address and send ACK, else reset USI
    case CHECK_ADDRESS: { // TODO auto assigment adderess
        bool rw = dataRegBuff & 0x01;
        dataRegBuff >>= 1;

        if((dataRegBuff == multicastAddress) || (dataRegBuff == slaveAddress)) {
            if(!rw) {
                overflowState = RECEIVE_DATA; // master want writing - receiving
                SET_USI_TO_SEND_ACK();
                break;
            }
            if((dataRegBuff == multicastAddress) && (slaveAddress != multicastAddress))  {
                SET_USI_TO_TWI_START_CONDITION_MODE();
                break;
            }
            overflowState = SEND_DATA;  // master want reading - transmitting
            SET_USI_TO_SEND_ACK();
            break;


        }
        SET_USI_TO_TWI_START_CONDITION_MODE();
        break;
    }

    /***************************************************************************************/

    // from here we just drop straight into USI_SLAVE_SEND_DATA if the
    // master sent an ACK
    // copy data from buffer to USIDR and set USI to shift byte
    case SEND_DATA: {
        int16_t tmp = requestCall(startCounter++);
        if(tmp >= 0)
            usi->data = tmp;
        else {
            SET_USI_TO_TWI_START_CONDITION_MODE();
            return;
        }
        overflowState = CHECK_ACK_FROM_SEND_DATA;
        SET_USI_TO_SEND_DATA();
        break;
    }

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
    if(server)
        return server->onReceiver(num, data);
    return ERR;
}

int16_t UsiTwiSlave::requestCall(uint8_t num)
{
    if(server)
        return server->onRequest(num);
    return ERR;
}
