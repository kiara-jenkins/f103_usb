
#define IMU_ADDR 0xD0	// MPU 9250, already shifted left, pin AD0 grounded

// pin PB9 = SDA
#define I2C_CLEAR_SDA	LL_GPIO_ResetOutputPin( GPIOB, LL_GPIO_PIN_9 );
#define I2C_SET_SDA	LL_GPIO_SetOutputPin(   GPIOB, LL_GPIO_PIN_9 );
#define I2C_READ_SDA	LL_GPIO_IsInputPinSet(  GPIOB, LL_GPIO_PIN_9 )
// pin PB8 = SCL
#define I2C_CLEAR_SCL	LL_GPIO_ResetOutputPin( GPIOB, LL_GPIO_PIN_8 );
#define I2C_SET_SCL 	LL_GPIO_SetOutputPin(   GPIOB, LL_GPIO_PIN_8 );

// #define I2C_DELAY DWT_Delay_us(5);	// 5us
// #define I2C_DELAY	tickdelay( 313 ); // 5*72 = 360 <==> 87kHz, 313 adjusted for 100kHz
#define I2C_DELAY	tickdelay( 60 ); // 1.25*72 = 90 <==> 263kHz, 60 adjusted for 333kHz

void I2C_init(void);

void I2C_start(void);

void I2C_restart(void);

void I2C_stop(void);

void I2C_write_bit(uint8_t b);

uint8_t I2C_read_SDA(void);

// Reading a bit in I2C:
uint8_t I2C_read_bit(void);

//_Bool I2C_write_byte(uint8_t B, _Bool start, _Bool stop);
_Bool I2C_write_1byte(uint8_t B);

//uint8_t I2C_read_byte(_Bool ack, _Bool stop);
uint8_t I2C_read_1byte( uint8_t ack );

//_Bool I2C_send_byte(uint8_t address, uint8_t data);

//uint8_t I2C_receive_byte(uint8_t address);

//_Bool I2C_send_byte_data(uint8_t address, uint8_t reg, uint8_t data);

//uint8_t I2C_receive_byte_data(uint8_t address, uint8_t reg);

//_Bool I2C_transmit(uint8_t address, uint8_t data[], uint8_t size);

//_Bool I2C_receive(uint8_t address, uint8_t reg[], uint8_t *data, uint8_t reg_size, uint8_t size);
