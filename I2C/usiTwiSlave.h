#ifndef _USI_TWI_SLAVE_H_
#define _USI_TWI_SLAVE_H_

#include <inttypes.h>

#define TWI_REQUIRE_STOP_CHEK true
#define TWI_REQUIRE_BUFFERS true
#define TWI_RX_BUFFER_SIZE 16
#define TWI_TX_BUFFER_SIZE 16


class UsiTwiSlave
{
public:
    void init(uint8_t address);

    void put(uint8_t);
    uint8_t get(void);

    uint8_t txWaitCount(void);
    uint8_t available(void);

    void checkStopAndReceiveCall();

    void onReceiveSetHandler(void (*)(uint8_t));
    void onRequestSetHandler(int16_t(*)());

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

    int16_t (*onRequest)(void);
    void (*onReceiver)(uint8_t);

    uint8_t slaveAddress;
    volatile TwiSlaveState overflowState;

    void SET_USI_TO_TWI_START_CONDITION_MODE();
    void SET_USI_TO_SEND_ACK();
    void SET_USI_TO_READ_ACK();
    void SET_USI_TO_SEND_DATA();
    void SET_USI_TO_READ_DATA();

    void receiveCall();
    int16_t requestCall();

    // void (*_onTwiDataRequest)(void);

    uint8_t txBuf[TWI_TX_BUFFER_SIZE];
    volatile uint8_t txHead;
    volatile uint8_t txTail;
    volatile uint8_t txCount;

    uint8_t rxBuf[TWI_RX_BUFFER_SIZE];
    volatile uint8_t rxHead;
    volatile uint8_t rxTail;
    volatile uint8_t rxCount;
    //FIFObuff txBuff, rxBuff;

    void putIntoRXBuff(uint8_t data);
    uint8_t getFromTXBuff();

private:
    UsiTwiSlave();
    UsiTwiSlave(const UsiTwiSlave &) {}
    UsiTwiSlave & operator=(UsiTwiSlave &) = default;
public:
    static UsiTwiSlave * getInstance();
    static void deleteInstance();
};





#endif
