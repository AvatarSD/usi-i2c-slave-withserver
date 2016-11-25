#ifndef _USI_TWI_SLAVE_H_
#define _USI_TWI_SLAVE_H_

#include <inttypes.h>


/***** api *****/
#define OK 0
#define ERR -1

typedef int8_t(*ReceiveHandler)(uint8_t, uint8_t);
typedef int16_t(*RequestHandler)(uint8_t);

class UsiTwiSlave
{
public:
    void init(uint8_t address);

    uint8_t getAddress();

    void onReceiveSetHandler(ReceiveHandler);
    void onRequestSetHandler(RequestHandler);

    static UsiTwiSlave * getInstance();

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

    static void startConditionVec();
    static void overflowVec();
    void startConditionHandler();
    void overflowHandler();

    uint8_t slaveAddress;
    volatile TwiSlaveState overflowState;

    void SET_USI_TO_TWI_START_CONDITION_MODE();
    void SET_USI_TO_SEND_ACK();
    void SET_USI_TO_READ_ACK();
    void SET_USI_TO_SEND_DATA();
    void SET_USI_TO_READ_DATA();

    uint8_t startCounter;

    int16_t requestCall(uint8_t);
    int8_t receiveCall(uint8_t num, uint8_t data);

    RequestHandler onRequest;
    ReceiveHandler onReceiver;

    UsiTwiSlave();
    UsiTwiSlave(const UsiTwiSlave &) {}
    UsiTwiSlave & operator=(UsiTwiSlave &) = default;

};





#endif
