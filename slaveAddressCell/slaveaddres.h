#ifndef SLAVEADDRES_H
#define SLAVEADDRES_H

#include <polymorphmemory.h>
#include <usiTwiSlave.h>

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


template<size_t additionalAddr = 0>
class SlaveAddress : public Composite<uint8_t>
{
public:

	static Error write(Address addr, uint8_t data, Num num)
	{
		newAddr = data;
		return OK;
	}
	static ReadType read(Address addr, Num num = 0)
	{
		if(newAddr != ERR)
			forAllsetAddress(newAddr);

		newAddr = ERR;
		return adress[0]->getAddress();
	}

	template<typename...Keepers>
	static void setAddreses(ISlaveAddress * keeper,
	                        Keepers * ...keepers)
	{
		adress[0] = keeper;

		static_assert(sizeof...(keepers) == additionalAddr,
		              "additional slave addresses pointer is not valid");

		_setAddreses(sizeof...(keepers), keepers...);
	}

private:
	template<typename...Keepers>
	static void _setAddreses(size_t amount,
	                         ISlaveAddress * keeper,
	                         Keepers * ... keepers)
	{
		adress[amount] = keeper;
		_setAddreses(sizeof...(keepers), keepers...);
	}
	static void _setAddreses(size_t amount) {}

	static void forAllsetAddress(uint8_t data)
	{
		for(uint8_t i = 0; i <= additionalAddr; i++)
			adress[i]->setAddress(data);
	}

	static int16_t newAddr;
	static ISlaveAddress * adress[additionalAddr + 1];
};

template<size_t additionalAddr>
int16_t SlaveAddress<additionalAddr>::newAddr = ERR;

template<size_t additionalAddr>
ISlaveAddress * SlaveAddress<additionalAddr>::adress[additionalAddr + 1];


#endif // SLAVEADDRES_H
