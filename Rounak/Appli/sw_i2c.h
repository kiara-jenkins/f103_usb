
#define IMU_ADDR 0xD0	// MPU 9250, already shifted left, pin AD0 grounded

// pin PB9 = SDA
#define I2C_CLEAR_SDA	LL_GPIO_ResetOutputPin( GPIOB, LL_GPIO_PIN_9 );
#define I2C_SET_SDA	LL_GPIO_SetOutputPin(   GPIOB, LL_GPIO_PIN_9 );
#define I2C_READ_SDA	LL_GPIO_IsInputPinSet(  GPIOB, LL_GPIO_PIN_9 )
// pin PB8 = SCL
#define I2C_CLEAR_SCL	LL_GPIO_ResetOutputPin( GPIOB, LL_GPIO_PIN_8 );
#define I2C_SET_SCL 	LL_GPIO_SetOutputPin(   GPIOB, LL_GPIO_PIN_8 );

#define I2C_DELAY	tickdelay( 60 );

// pin PB9 = SDA
__STATIC_INLINE void I2C_sda0()	{ LL_GPIO_ResetOutputPin( GPIOB, LL_GPIO_PIN_9 ); }
__STATIC_INLINE void I2C_sda1()	{ LL_GPIO_SetOutputPin(   GPIOB, LL_GPIO_PIN_9 ); }
// Note : LL_GPIO_IsInputPinSet() returns 0 or 1
__STATIC_INLINE uint32_t I2C_read_sda() { return LL_GPIO_IsInputPinSet(  GPIOB, LL_GPIO_PIN_9 ); }

// pin PB8 = SCL
__STATIC_INLINE void I2C_scl0()	{ LL_GPIO_ResetOutputPin( GPIOB, LL_GPIO_PIN_8 ); }
__STATIC_INLINE void I2C_scl1()	{ LL_GPIO_SetOutputPin(   GPIOB, LL_GPIO_PIN_8 ); }

// #define I2C_TICKS	313	// 5*72 = 360 <==> 87kHz, 313 adjusted for 100kHz
#define I2C_TICKS	60	// 1.25*72 = 90 <==> 263kHz, 60 adjusted for 333kHz

// #define I2C_DELAY DWT_Delay_us(5);	// 5us
__STATIC_INLINE void I2C_delay() { tickdelay( I2C_TICKS ); }

/*	 S	bit   ack/nack     P
	_      ______ _____         __
    sda	 |____|______|_____|_______|
	___       __    __      ______
    scl	   |_____|  |__|  |____|

- slave ignore les transitions de SDA quand SCL est 0
- les transitions de SDA avec SCL = 1 sont interpretees comme Start 'S' et Stop 'P'
- 9 pulses positives de ck par byte utile (8 + ack|nak)
	- le front montant échantillonne SDA
	- le front descendant
		- invite le slave a actionner SDA s'il en a le controle, ou le relacher s'il a fini
		- precede l'action du master sur SDA (cas particulier : front descendant SCL du start aussi)
- le master doit envoyer une pulse de SCL pour le dernier Ack/Nak avant le Stop, son front descendant precede
  la mise a zero de SDA necessaire pour le Stop (en fait le Stop coute un bit de plus)

	      P    S                reStart
	       ____                _____
    sda	______|    |______     ___|     |______
	    _________                _____
    scl	___|         |____     _____|     |____

- comparaison entre Stop-Start et re-Start :
	- dans les 2 cas le front montant de SCL va causer le sampling d'un bit sans signification
	- le front montant de SDA :
		- va compter comme Stop dans le cas Stop-Start
		- va etre ignoré dans le cas re-Start
  conclusion : on comprend pourquoi certains ne se donnent pas le mal d'implementer le restart
*/

__STATIC_INLINE void I2C_start()
{				// normalement SCL et SDA sont a 1 (idle)
I2C_scl1();	I2C_delay();	// sinon on envoie un Stop
I2C_sda1();	I2C_delay();
I2C_sda0();	I2C_delay();	// le Start commence ici
I2C_scl0();	I2C_delay();
}

__STATIC_INLINE void I2C_restart(void)
{				// normalement SCL est 0 et SDA est libre (ack a ete lu)
I2C_scl0();			// pour etre sur que le Ack est fini
I2C_sda1();	I2C_delay();	// SDA doit etre libre
I2C_scl1();	I2C_delay();	// idle state
I2C_sda0();	I2C_delay();	// le Start commence ici
I2C_scl0();	I2C_delay();
}

__STATIC_INLINE void I2C_stop(void)
{
I2C_sda0();	I2C_delay();
I2C_scl1();	I2C_delay();
I2C_sda1();	I2C_delay();
}

// write a bit with 1 SCL pulse (call this just after a SCL falling edge)
__STATIC_INLINE void I2C_write_bit( uint32_t b )
{
if (b) I2C_sda1(); else I2C_sda0();
I2C_delay();	I2C_scl1();
I2C_delay();	I2C_scl0();
}

// read a bit with 1 SCL pulse (call this just after a SCL falling edge)
__STATIC_INLINE uint32_t I2C_read_bit(void)
{
uint32_t b;
I2C_sda1();	I2C_delay();	// let slave control SDA
I2C_scl1();	I2C_delay();
b = I2C_read_sda();		// sample just before falling edge
I2C_scl0();
return b;
}


// write 1 byte, return ack from slave
uint32_t I2C_write_1byte( uint32_t B);

// read 1 byte, give ack to the slave
uint32_t I2C_read_1byte( uint32_t ack );

/// Register level (8-bit register address)
// Transactions include Start & Stop

// write N bytes, return number of bytes written
uint32_t I2C_transaction_write_N_regs( uint32_t reg_addr, uint8_t * buf, uint32_t N );

// read N bytes, return number of bytes actually got
uint32_t I2C_transaction_read_N_regs( uint32_t reg_addr, uint8_t * buf, uint32_t N );

// read N 16-bit words, each one from 2 registers in big endian order, return number of words got
uint32_t I2C_transaction_read_N_words_16be( uint32_t reg_addr, uint16_t * buf, uint32_t N );
