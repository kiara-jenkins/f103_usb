
extern      uint8_t i2c_addr;
extern volatile uint8_t i2c_rxcnt;
extern volatile uint8_t i2c_rxbuf[16];


#ifdef __cplusplus
extern "C" {
#endif

void i2c_init(void);
void i2c_enable(void);

void i2c_start( int cnt );

#ifdef __cplusplus
}
#endif


