#ifndef _USI_TWI_SLAVE_H_
#define _USI_TWI_SLAVE_H_

#include <inttypes.h>

void usiTwiSlaveInit(uint8_t);

void usiTwiTransmitByte(uint8_t);
uint8_t usiTwiReceiveByte(void);

bool usiTwiDataInReceiveBuffer(void);
extern void (*_onTwiDataRequest)(void);
bool usiTwiDataInTransmitBuffer(void);
uint8_t usiTwiAmountDataInReceiveBuffer(void);

// on_XXX handler pointers
extern void (*usi_onRequestPtr)(void);
extern void (*usi_onReceiverPtr)(uint8_t);

//driver buffer definitions
// permitted buffer sizes: 1, 2, 4, 8, 16, 32, 64, 128 or 256

#ifndef TWI_RX_BUFFER_SIZE
#define TWI_RX_BUFFER_SIZE (16)
#endif
#define TWI_RX_BUFFER_MASK (TWI_RX_BUFFER_SIZE - 1)

#if (TWI_RX_BUFFER_SIZE & TWI_RX_BUFFER_MASK)
#error TWI RX buffer size is not a power of 2
#endif

#ifndef TWI_TX_BUFFER_SIZE
#define TWI_TX_BUFFER_SIZE (16)
#endif
#define TWI_TX_BUFFER_MASK (TWI_TX_BUFFER_SIZE - 1)

#if (TWI_TX_BUFFER_SIZE & TWI_TX_BUFFER_MASK)
#error TWI TX buffer size is not a power of 2
#endif

#endif
