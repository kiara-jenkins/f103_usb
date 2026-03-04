#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_i2c.h"

#include "i2c.h"
#include "options.h"

#ifdef USE_I2C

// static storage
         uint8_t i2c_addr;
volatile uint8_t i2c_rxcnt;
volatile uint8_t i2c_rxbuf[16];
volatile uint8_t i2c_rxi = 0;

// local functions prototypes
static void i2c_Master_Reception_Callback(void);
static void i2c_Master_Complete_Callback(void);
static void Error_Callback(void);

// Interrupt handlers

// I2C1 (Master) interrupt
void I2C1_EV_IRQHandler(void)
{
// SB = Start Bit flag in ISR register
if	( LL_I2C_IsActiveFlag_SB(I2C1) )
	{
	// Slave address | 1 for a read request
	LL_I2C_TransmitData8( I2C1, i2c_addr | 1 );
	}
// Address sent flag in ISR register
else if	( LL_I2C_IsActiveFlag_ADDR(I2C1) )
	{
	if	( i2c_rxcnt == 1 )
		{
		// Prepare the generation of a Non ACKnowledge condition after next received byte
		LL_I2C_AcknowledgeNextData( I2C1, LL_I2C_NACK );
		// Enable Buffer Interrupts
		LL_I2C_EnableIT_BUF(I2C1);
		}
	else if	( i2c_rxcnt == 2 )
		{
		// Prepare the generation of a Non ACKnowledge condition after next received byte
		LL_I2C_AcknowledgeNextData( I2C1, LL_I2C_NACK );	
		// Enable Pos
		LL_I2C_EnableBitPOS(I2C1);
		}
	else	{
		// Enable Buffer Interrupts
		LL_I2C_EnableIT_BUF(I2C1);
		}
	// Clear ADDR flag in ISR register
	LL_I2C_ClearFlag_ADDR(I2C1);
	}
// Byte Transfer Finished flag in ISR register
else if	( LL_I2C_IsActiveFlag_BTF(I2C1) )
	{
	// Call function Master Complete Callback
	i2c_Master_Complete_Callback();
	}
// Receive data register Not Empty flag in ISR register
else if	( LL_I2C_IsActiveFlag_RXNE(I2C1) )
	{
	// Call function Master Reception Callback
	i2c_Master_Reception_Callback();
	}
}

// I2C1 (Master) error interrupt
void I2C1_ER_IRQHandler(void)
{
// Call Error function
Error_Callback();
}

// // i2c API

// I2C1 init
void i2c_init(void)
{
LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);

// Configure NVIC for I2C1 event
NVIC_SetPriority( I2C1_EV_IRQn, 3 );  
NVIC_EnableIRQ(   I2C1_EV_IRQn );
// Configure NVIC for I2C1 error
NVIC_SetPriority( I2C1_ER_IRQn, 7 );  
NVIC_EnableIRQ(   I2C1_ER_IRQn );

// Disable I2C1 prior modifying configuration registers
LL_I2C_Disable(I2C1);

// Configure the SCL Clock Speed 400 KHz (assuming PCLK1 = 36MHz, the maximum)
LL_I2C_ConfigSpeed( I2C1, 36000000, 100000, LL_I2C_DUTYCYCLE_2 );
}

void i2c_enable(void)
{
// Enable I2C1
LL_I2C_Enable(I2C1);
// Enable Events interrupts
// Enable Errors interrupts
LL_I2C_EnableIT_EVT(I2C1);
LL_I2C_EnableIT_ERR(I2C1);
}

// start read transaction
void i2c_start( int cnt )
{
i2c_rxcnt = cnt;
i2c_rxi   = 0;

// Prepare the generation of a ACKnowledge or Non ACKnowledge condition after the address rx or next received byte
LL_I2C_AcknowledgeNextData( I2C1, LL_I2C_ACK );

LL_I2C_GenerateStartCondition(I2C1);
}


// Function called from I2C IRQ Handler when RXNE flag is set
// in charge of reading byte received on I2C
static void i2c_Master_Reception_Callback(void)
{
if	( i2c_rxcnt > 3 )
	{
	// Read byte in Receive Data register, RXNE flag is cleared
	i2c_rxbuf[i2c_rxi++] = LL_I2C_ReceiveData8(I2C1);
	i2c_rxcnt--;
	}
else if	( ( i2c_rxcnt == 2 ) || ( i2c_rxcnt == 3 ) )
	{
	// Disable Buffer Interrupts
	LL_I2C_DisableIT_BUF(I2C1);
	}
else if	( i2c_rxcnt == 1 )
	{
	// Disable Buffer Interrupts
	LL_I2C_DisableIT_BUF(I2C1);
	// Generate Stop condition
	LL_I2C_GenerateStopCondition(I2C1);
	// Read byte in Receive Data register, RXNE flag is cleared
	i2c_rxbuf[i2c_rxi++] = LL_I2C_ReceiveData8(I2C1);
	i2c_rxcnt--;
	// Call function Master Complete Callback
	i2c_Master_Complete_Callback();
	}
}

// Function called from I2C IRQ Handler when STOP flag is set
// in charge of checking data received,
static void i2c_Master_Complete_Callback(void)
{
if	( i2c_rxcnt == 3 )
	{
	// Prepare the generation of a Non ACKnowledge condition after next received bytes
	LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_NACK);

	// Read byte in Receive Data register, RXNE flag is cleared
	i2c_rxbuf[i2c_rxi++] = LL_I2C_ReceiveData8(I2C1);
	i2c_rxcnt--;

	// Disable Buffer Interrupts
	LL_I2C_DisableIT_BUF(I2C1);
	}
else if	( i2c_rxcnt == 2 )
	{
	// Generate Stop condition
	LL_I2C_GenerateStopCondition(I2C1);

	// Read byte in Receive Data register, RXNE flag is cleared
	i2c_rxbuf[i2c_rxi++] = LL_I2C_ReceiveData8(I2C1);
	i2c_rxcnt--;

	// Read byte in Receive Data register, RXNE flag is cleared
	i2c_rxbuf[i2c_rxi++] = LL_I2C_ReceiveData8(I2C1);
	i2c_rxcnt--;
	}
else	{
	if	( i2c_rxcnt > 0 )
		{
		// Read byte in Receive Data register, RXNE flag is cleared
		i2c_rxbuf[i2c_rxi++] = LL_I2C_ReceiveData8(I2C2);
		i2c_rxcnt--;
		}
	}

if	( i2c_rxcnt == 0 )
	{
	LL_I2C_DisableIT_EVT(I2C1);
	LL_I2C_DisableIT_ERR(I2C1);

	// success
	i2c_rxcnt = 7;	// for debug
	// LED_On();
	}
}

static void Error_Callback(void)
{
NVIC_DisableIRQ(I2C1_EV_IRQn);
NVIC_DisableIRQ(I2C1_ER_IRQn);
}

#endif
