#ifndef SLAVEADDRES_H
#define SLAVEADDRES_H

//#include <polymorphmemory.h>
//#include <usiTwiSlave.h>

#include "../polymorphmemory/polymorphmemory.h"
#include "../I2C/usiTwiSlave.h"





template<size_t additionalAddr = 0>
class AddressKeeper : public ISlaveAddress, public IMulticastAddress
{
public:

	template <ISlaveAddress * ... rest>
	AddressKeeper(IMulticastAddress * addr, ISlaveAddress * n,
	              ISlaveAddress * ... rest) :
	    multicastAddrKpr(addr), AddressKeeper(additionalAddr, rest...)
	{
		static_assert(sizeof...(rest) == additionalAddr, "param amaunt is not valid");

		addrs[0];

	}

private:

	template <ISlaveAddress * ... rest>
	AddressKeeper(int amount, ISlaveAddress * n,
	              ISlaveAddress * ... rest): AddressKeeper(--amount, rest...)
	{
		addrs[amount] = n;
	}
	AddressKeeper(int amount) {}


public:


	void setAddress(uint8_t addr)
	{
		for(uint8_t i = 0; i <= additionalAddr; i++)
			addrs[i] = addr;
	}
	uint8_t getAddress()
	{
		return addrs[0]->getAddress();
	}
	uint8_t getMulticastAddress()
	{
		return multicastAddrKpr->getMulticastAddress();
	}

private:
	ISlaveAddress * addrs[additionalAddr + 1];
	IMulticastAddress * multicastAddrKpr;
};




template<>
class SlaveAddress : public Composite<uint8_t>
{
public:

	//template<>
	SlaveAddress(IMulticastAddress * multicastAddrKpr,
	             ISlaveAddress * adressKeeper,
	             ISlaveAddress * ... adressKeepers) //: keepersSize(sizeof...(adressKeepers))
	{

	}


	Error write(Address addr, uint8_t data, Num num)
	{
		/*if address is multicast - reset slaveAddress to multicast value immediatly*/
		if(data == keeper.getMulticastAddress()) {
			keeper.setAddress(data);
			newAddr = ERR;
			return OK;
		}

		/*
		 * todo: newAddr to ERR at second read(when first read wasn't be here - slave
		 * address will not change).
		 * First version without possibility to change slave addres selectively.
		 *
		 * Algoritm:
		 * 1. Write ${MULTICAST_ADDRESS} by ${MULTICAST_ADDRESS} in to the SlaveAddress cell.
		 *    All devices will must have ${MULTICAST_ADDRESS} address.
		 * 2. Now start the autoaddress procedure at each devices:
		 *    2.1. Check devices accebility by checking response at I2C line by
		 *         ${MULTICAST_ADDRESS} address. if no - nothing device with
		 *         ${MULTICAST_ADDRESS} available at line.
		 *    2.2. Check possibility to read by ${MULTICAST_ADDRESS}. If no - all devices
		 *         alredy have unique addres different from ${MULTICAST_ADDRESS}.
		 *         Autoadress procedure has been finished.
		 *    2.3. Check whether free cell, which we want to appoint to the device,
		 *         which will be "arbitration winner". For examble for first device
		 *         it will be the 0x01 cell.
		 *    2.4. Write the selected cell number by ${MULTICAST_ADDRESS} in to the
		 *         SlaveAddress cell. At all devices, newAddr variable will be contain
		 *         writed cell number.
		 *    2.5. Start consequentially reading before several cells of SlaveAddress cell for
		 *         arbitration procedure. Reading must be consequentially, because we must
		 *         read slave address cell only in arbitration winner. Fot example:
		 *         SlaveAddress cell at 0x10 address. Before it we have 0x0f bytes of
		 *         unique uuid number at each device. When we start reading this cells
		 *         by ${MULTICAST_ADDRESS} we get arbitrations: each device, which lose
		 *         arbitrations will wait next start contitions. When we read the SlaveAddress
		 *         cell at winner device - it will change own slave address and prohibit the
		 *         furure reading(will not participate in future arbitration for remaining
		 *         devices) while slave addres will change to multicast again.
		 *    2.6. Repeat this procedure
		 *
		 */

		newAddr = data;
		return OK;
	}
	ReadType read(Address addr, Num num = 0)
	{
		if(newAddr != ERR)
			keeper.setAddress(newAddr);

		newAddr = ERR;
		return keeper.getAddress();
	}

private:
//	uint8_t keepersSize;
	int16_t newAddr;
	AddressKeeper<sizeof...(adressKeepers)> keeper;
};


#endif // SLAVEADDRES_H
