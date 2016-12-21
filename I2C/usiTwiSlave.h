#ifndef _USI_TWI_SLAVE_H_
#define _USI_TWI_SLAVE_H_

#include <inttypes.h>
#include <usi.h>


/***** api *****/
#define OK 0
#define ERR -1

typedef uint8_t I2CAddress;

class ISlaveAddress
{
public:
    virtual void setAddress(I2CAddress addr);
    virtual I2CAddress getAddress() const;
};

class IMulticastAddress
{
public:
    virtual I2CAddress getMulticastAddress() const;
};

class IServer
{
public:
    virtual int8_t onReceiver(uint8_t, uint8_t)
    {
        return ERR;
    }
    virtual int16_t onRequest(uint8_t)
    {
        return ERR;
    }
};

class ITwiSlave : public ISlaveAddress, public IMulticastAddress
{
public:
    ITwiSlave(ISlaveAddress * memory);

    virtual void init();
    virtual void onEventHandler(IServer * server);

protected:
    ISlaveAddress * memory;
};

//for testing routine
class SlaveAddressKeeper : public ISlaveAddress
{
public:
    AddrMemory(I2CAddress addr) : addr(addr) {}
    void setAddress(I2CAddress addr)
    {
        this->addr = addr;
    }
    I2CAddress getAddress() const
    {
        return this->addr;
    }
private:
    I2CAddress addr;
};

class UsiTwiSlave : public IUsiEvent, public ITwiSlave
{
public:
    UsiTwiSlave(USI * usi,
                ISlaveAddress * memory,
                I2CAddress multicastAdress);

    void init() final;
    void onEventHandler(IServer * server) final;

    void setAddress(I2CAddress addr) final;
    I2CAddress getAddress() const final;
    I2CAddress getMulticastAddress() const final;

    // port side handlers
    void startConditionHandler() final;
    void overflowHandler() final;

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

    IServer * server; //upper
    USI * usi;        //lower

    I2CAddress slaveAddress;
    const I2CAddress multicastAddress;

    volatile TwiSlaveState overflowState;
    volatile uint8_t startCounter;

    void SET_USI_TO_TWI_START_CONDITION_MODE();
    void SET_USI_TO_SEND_ACK();
    void SET_USI_TO_READ_ACK();
    void SET_USI_TO_SEND_DATA();
    void SET_USI_TO_READ_DATA();

    // soft side handlers
    int16_t requestCall(uint8_t);
    int8_t receiveCall(uint8_t num, uint8_t data);
};

#endif
