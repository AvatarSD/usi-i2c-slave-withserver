#ifndef _USI_TWI_SLAVE_H_
#define _USI_TWI_SLAVE_H_

#include <inttypes.h>

/**** Settings ****/
#define TWI_REQUIRE_STOP_CHEK 0
#define TWI_REQUIRE_BUFFERS 0
#define TWI_RX_BUFFER_SIZE 16
#define TWI_TX_BUFFER_SIZE 16


/***** api *****/
#define OK 0
#define ERR -1

typedef int8_t(*ReceiveHandler)(uint8_t, uint8_t);
typedef int16_t(*RequestHandler)(uint8_t);

class UsiTwiSlave
{
public:
    void init(uint8_t address);

#if TWI_REQUIRE_BUFFERS
    void put(uint8_t);
    uint8_t get(void);

    uint8_t txWaitCount(void);
    uint8_t available(void);

    void onReceiveSetHandler(void (*)(uint8_t));
    void onRequestSetHandler(void(*)());
#else
    void onReceiveSetHandler(ReceiveHandler); //int16_t (*)(uint8_t, uint8_t));
    void onRequestSetHandler(RequestHandler);
#endif

#if TWI_REQUIRE_STOP_CHEK
    void checkStopAndReceiveCall();
#endif
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

#if TWI_REQUIRE_BUFFERS
    void receiveCall();
    void requestCall();

    void (*onRequest)(void);
    void (*onReceiver)(uint8_t);

    uint8_t txBuf[TWI_TX_BUFFER_SIZE];
    volatile uint8_t txHead;
    volatile uint8_t txTail;
    volatile uint8_t txCount;

    uint8_t rxBuf[TWI_RX_BUFFER_SIZE];
    volatile uint8_t rxHead;
    volatile uint8_t rxTail;
    volatile uint8_t rxCount;


    void putIntoRXBuff(uint8_t data);
    uint8_t getFromTXBuff();
#else
    uint8_t startCounter;

    int16_t requestCall(uint8_t);
    int8_t receiveCall(uint8_t num, uint8_t data);

    RequestHandler onRequest;
    ReceiveHandler onReceiver;
#endif

private:
    UsiTwiSlave();
    UsiTwiSlave(const UsiTwiSlave &) {}
    UsiTwiSlave & operator=(UsiTwiSlave &) = default;
public:
    static UsiTwiSlave * getInstance();
};





#endif
