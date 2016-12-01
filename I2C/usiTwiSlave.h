#ifndef _USI_TWI_SLAVE_H_
#define _USI_TWI_SLAVE_H_

#include <inttypes.h>
#include <usi.h>


/***** api *****/
#define OK 0
#define ERR -1


class iServer
{
public:
    virtual int16_t onRequest(uint8_t);
    virtual int8_t onReceiver(uint8_t, uint8_t);
};


class UsiTwiSlave : public iUSIcallback
{
public:

    UsiTwiSlave(USI * usi);

    void init(iServer * server, uint8_t address, uint8_t multicastAddress);

    uint8_t getAddress();
    void setAddress(uint8_t addr);

    uint8_t getMulticastAddress();
    void setMulticastAddress(uint8_t addr);

    void onEventHandler(iServer * server);

private:
    enum TwiSlaveState {
        CHECK_ADDRESS = 0x00,

        //data sending cycle
        SEND_DATA = 0x01,
        CHECK_ACK_FROM_SEND_DATA = 0x02,
        CHECK_REPLY_FROM_ACK = 0x03,

        //data resiving sycle
        RECEIVE_DATA = 0x04,
        GET_DATA_AND_SEND_ACK = 0x05
    };

    USI * usi;
    iServer * server;

    uint8_t slaveAddress;
    uint8_t multicastAddress;

    volatile TwiSlaveState overflowState;
    volatile uint8_t startCounter;

    void SET_USI_TO_TWI_START_CONDITION_MODE();
    void SET_USI_TO_SEND_ACK();
    void SET_USI_TO_READ_ACK();
    void SET_USI_TO_SEND_DATA();
    void SET_USI_TO_READ_DATA();

    // port side handlers
    void startConditionHandler() final;
    void overflowHandler() final;

    // soft side handlers
    int16_t requestCall(uint8_t);
    int8_t receiveCall(uint8_t num, uint8_t data);

};





#endif
