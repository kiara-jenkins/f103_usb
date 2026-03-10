#include "options.h"
#ifdef USE_I2C

#include "stm32f1xx_ll_gpio.h"
#include "sys.h"
#include "sw_i2c.h"

///
///	Byte level
///
// write 1 byte, return true if ack from slave
uint32_t I2C_write_1byte( uint32_t B )
{
uint32_t ack = 0;
for	( uint32_t i = 0; i < 8; i++ )
	{
        I2C_write_bit( B & 0x80 ); // write the most-significant bit
        B <<= 1;
	}
ack = I2C_read_bit();
return !ack; // return true if ack
}

// read 1 byte, give ack to the slave
uint32_t I2C_read_1byte( uint32_t ack )
{
uint32_t B = 0;
for	( uint32_t i = 0; i < 8; i++ )
	{
        B <<= 1;
        B |= I2C_read_bit();
	}
if	(ack)
        I2C_write_bit(0);
else
        I2C_write_bit(1);
return B;
}

///
///	Register level (8-bit register address)
///

// Transactions include Start & Stop

// write N bytes, return number of bytes written
uint32_t I2C_transaction_write_N_regs( uint32_t slave_addr, uint32_t reg_addr, uint32_t N, uint8_t * buf )
{
I2C_start();
if	( !I2C_write_1byte( slave_addr ) )
	{ I2C_stop(); return 0; }
if	( !I2C_write_1byte( reg_addr ) )
	{ I2C_stop(); return 0; }
uint32_t i;
for	( i = 0; i < N; i++ )
	{
	if	( !I2C_write_1byte( buf[i] ) )
		return i;
	}
I2C_stop();
return i;
}

// read N bytes, return number of bytes actually got
uint32_t I2C_transaction_read_N_regs( uint32_t slave_addr, uint32_t reg_addr, uint32_t N, uint8_t * buf )
{
I2C_start();
if	( !I2C_write_1byte( slave_addr ) )
	{ I2C_stop(); return 0; }
if	( !I2C_write_1byte( reg_addr ) )
	{ I2C_stop(); return 0; }
I2C_restart();
if	( !I2C_write_1byte( slave_addr | 1 ) )  // addr+1 -> I2C read
	{ I2C_stop(); return 0; }
uint32_t i;
for	( i = 0; i < (N-1); i++ )
	buf[i] = I2C_read_1byte( 1 );
buf[i] = I2C_read_1byte( 0 );		// last one, no ack
I2C_stop();
return ++i;
}

// read N 16-bit words, each one from 2 registers in big endian order, return number of words got
uint32_t I2C_transaction_read_N_words_16be( uint32_t slave_addr, uint32_t reg_addr, uint32_t N, uint16_t * buf )
{
I2C_start();
if	( !I2C_write_1byte( slave_addr ) )
	{ I2C_stop(); return 0; }
if	( !I2C_write_1byte( reg_addr ) )
	{ I2C_stop(); return 0; }
I2C_restart();
if	( !I2C_write_1byte( slave_addr | 1 ) )  // addr+1 -> I2C read
	{ I2C_stop(); return 0; }
uint32_t i;
for	( i = 0; i < (N-1); i++ )
	{
	buf[i]  = I2C_read_1byte( 1 ) << 8;
	buf[i] |= I2C_read_1byte( 1 );
	}
buf[i]  = I2C_read_1byte( 1 ) << 8;
buf[i] |= I2C_read_1byte( 0 );		// last byte, no ack
I2C_stop();
return ++i;
}

#endif
