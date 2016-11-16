#ifndef _USI_TWI_SLAVE_H_
#define _USI_TWI_SLAVE_H_

#include <inttypes.h>



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

    void (*onRequest)(void);
    void (*onReceiver)(uint8_t);

private:
    enum TwiSlaveState {
        USI_SLAVE_CHECK_ADDRESS = 0x00,
        USI_SLAVE_SEND_DATA = 0x01,
        USI_SLAVE_REQUEST_REPLY_FROM_SEND_DATA = 0x02,
        USI_SLAVE_CHECK_REPLY_FROM_SEND_DATA = 0x03,
        USI_SLAVE_REQUEST_DATA = 0x04,
        USI_SLAVE_GET_DATA_AND_SEND_ACK = 0x05
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

    void USI_RECEIVE_CALLBACK();
    void ONSTOP_USI_RECEIVE_CALLBACK();
    void USI_REQUEST_CALLBACK();


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
