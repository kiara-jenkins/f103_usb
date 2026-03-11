
/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_system.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_usart.h"
#include "stm32f1xx_hal_pcd.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_hid.h"

#include "options.h"
#include "sys.h"
#include "gpio.h"
#include "uarts.h"
#include "encoder.h"
#include "sw_i2c.h"
#include "adc.h"
#include "CDC.h"
#include "mpu9250_constants.h"

// // static data
USBD_HandleTypeDef hUsbDeviceFS;
extern PCD_HandleTypeDef hpcd_USB_FS;

volatile unsigned int cnt100Hz = 0;
volatile unsigned int cnt1Hz = 0;
volatile unsigned int cntblinks = 1;

// MIDI Message arrays: 4 Bytes
// 1st byte : MSBs --> Cable Number 0, LSBs --> duplicate MSBs of MIDI opcode (!??!)
uint8_t midiNoteOn[4]  = { 0x09, 0x93, 65, 99 };	// canal 3 comme exemple
uint8_t midiNoteOff[4] = { 0x08, 0x83, 65, 0  };

// double MIDI message
// #define DOUBLE_EVENT
uint8_t midiNoteOffOn[8]  = { 0x08, 0x83, 65, 0, 0x09, 0x93, 65, 99 };

// B0 7B 00 (123) = all notes off
uint8_t midiAllOff[4]= { 0x0b, 0xb0, 0x7b, 0 };

// // interrupt routines
#ifdef __cplusplus
extern "C" {
#endif
// systick interrupt handler
void SysTick_Handler()
{
++cnt100Hz;
#ifdef BLINK	// sur nucleo utilisant SPI1, la LED est masquee par SCK, alors renoncer au blink
	{	// 1 to 5 LED blinks per second
	switch	( cnt100Hz % 100 )
		{
		case 0 : ++cnt1Hz; if ( cntblinks ) LED_ON();
			break;
		case 15 : if ( cntblinks > 1 ) LED_ON();
			break;
		case 30 : if ( cntblinks > 2 ) LED_ON();
			break;
		case 45 : if ( cntblinks > 3 ) LED_ON();
			break;
		case 60 : if ( cntblinks > 4 ) LED_ON();
			break;
		case 75 : if ( cntblinks > 5 ) LED_ON();
			break;
		case 4 :
		case 19 :
		case 34 :
		case 49 :
		case 64 :
		case 79 : LED_OFF();
			break;
		}
	}
#else	// si BLINK, c'est deja fait ci-dessus
if	( ( cnt100Hz % 100 ) == 0 )
	++cnt1Hz;
#endif
} // SysTick_Handler()


#ifdef MIDI_USB
// USB interrupt routine
void USB_LP_CAN1_RX0_IRQHandler(void)
{
HAL_PCD_IRQHandler(&hpcd_USB_FS);
}
#endif

#ifdef __cplusplus
}
#endif

#ifdef USE_CDC
void cmd_handler( int c )
{
switch	( c )
	{
	case '#' :
		CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk;	// p. C1-24
		CoreDebug->DEMCR |=  CoreDebug_DEMCR_TRCENA_Msk;
		DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk;
		DWT->CTRL |=  DWT_CTRL_CYCCNTENA_Msk;
		DWT->CYCCNT = 0;
		__ASM volatile ("NOP");
		__ASM volatile ("NOP");
		__ASM volatile ("NOP");
		tickdelay( 5000 );
		CDC_printf( "DWT running ? %u\n", DWT->CYCCNT );
		break;
	case '&' :
		CDC_printf( "HCLK %u, DWT %u\n", SystemCoreClock, DWT->CYCCNT );
		DWT->CYCCNT = 0;
		break;
	case '$' :
		report_interrupts();
		break;
	#ifdef ENCODER_TIM
	case 'e' :
		CDC_printf( "b=%d, xy=%d:%d, e=%u\n", LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_13 ),
			LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_8 ), LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_9 ), encoder_get(TIM1) );
		break;
	#endif

	#ifdef USE_ADC_4CH
	case 'k' :
		adc_timer_stop();
		CDC_printf("adc timer stopped\n");
		break;
	case 't' :
		adc_timer_init( SystemCoreClock / ADCFREQ );
		CDC_printf("adc timer started\n");
		break;
	case 'K' :
		adc_timer_stop();
		adc_calib();
		adc_timer_init( SystemCoreClock / ADCFREQ );
		CDC_printf( "Stop-Calib-Start Done\n" );
		break;
	case 'u' :
		adc_uncalib();
		CDC_printf("adc uncalibrated\n");
		break;
	#endif

	#ifdef MIDI_USB
	case 's' :
		/* vu dans usbd_def.h
		#define USBD_STATE_DEFAULT                              0x01U
    		#define USBD_STATE_ADDRESSED                            0x02U
		#define USBD_STATE_CONFIGURED                           0x03U
		#define USBD_STATE_SUSPENDED                            0x04U
		c'est hUsbDeviceFS.dev_state !! eureka !!
		*/
		CDC_printf("status %u %u %u %u %u\n",
			hUsbDeviceFS.dev_config_status,
			hUsbDeviceFS.dev_state,
			hUsbDeviceFS.dev_connection_status,
			hUsbDeviceFS.ep_out[1].status,
			((USBD_HID_HandleTypeDef *) hUsbDeviceFS.pClassData)->state
			);
		break;
	#endif

	case '0' :
	case '1' :
	case '2' :
	case '3' :
	case '4' :
	case '5' : cntblinks = c - '0';
		break;

	#ifdef USE_I2C
	case 'a' : {	// test MPU 9250
		int ak;
		I2C_start(); ak = I2C_write_1byte( MPU9250_ADDRESS ); I2C_stop();
		CDC_printf("addr : %d \n", ak );
		} break;
	case 'b' : {
		int ak;
		I2C_start(); ak = I2C_write_1byte( 0xC8 ); I2C_stop();
		CDC_printf("addr : %d \n", ak );
		} break;
	case 'R' : {	// lecture du registre WHO_AM_I avec restart (expect 0x71)
		uint32_t vu;
		uint8_t va;
		vu = I2C_transaction_read_N_regs( MPU9250_ADDRESS, WHO_AM_I_MPU9250, 1, &va );
		CDC_printf("read %d bytes from reg WHO_AM_I_MPU9250 (%02x)\n", vu, va );
		} break;
	case 'Q' : {	// lecture du registre WHO_AM_I avec stop-start (expect 0x71)
		I2C_start(); I2C_write_1byte( MPU9250_ADDRESS );
		if	( I2C_write_1byte( WHO_AM_I_MPU9250 ) )
			{
			I2C_stop(); I2C_start();
			if	( I2C_write_1byte( MPU9250_ADDRESS + 1 ) )
				{
				int va = I2C_read_1byte( 0 ); I2C_stop();
				CDC_printf("reg WHO_AM_I_MPU9250 = 0x%02x\n", va );
				}
			else	CDC_printf("restart BAD\n");
			}
		else	CDC_printf("reg adr WHO_AM_I_MPU9250 NAK\n");
		} break;
	case 'w' : {	// lecture du registre WOM_Threshold avec restart
		uint32_t vu;
		uint8_t va;
		vu = I2C_transaction_read_N_regs( MPU9250_ADDRESS, WOM_THR, 1, &va );
		CDC_printf("read %d bytes from reg WOM_THR (%02x)\n", vu, va );
		} break;
	case 'W' : {	// ecriture du registre WOM_Threshold
		uint32_t vu;
		uint8_t va = 0x70 + cntblinks;
		vu = I2C_transaction_write_N_regs( MPU9250_ADDRESS, WOM_THR, 1, &va );
		CDC_printf("writen %d bytes to reg WOM_THR (%02x)\n", vu, va );
		} break;
	case 'A' : {	// lecture accelerometers
		uint32_t vu;
		int16_t va[3];
		vu = I2C_transaction_read_N_words_16be( MPU9250_ADDRESS, ACCEL_XOUT_H, 3, (uint16_t *)va );
		CDC_printf("got %d values from accel (%5d %5d %5d)\n", vu, va[0], va[1], va[2] );
		} break;
	case 'G' : {	// lecture gyros
		uint32_t vu;
		int16_t va[3];
		vu = I2C_transaction_read_N_words_16be( MPU9250_ADDRESS, GYRO_XOUT_H, 3, (uint16_t *)va );
		CDC_printf("got %d values from gyro (%5d %5d %5d)\n", vu, va[0], va[1], va[2] );
		} break;
	case 'm' : {	// lecture ID magneto (expect 0x48)
		uint32_t vu;
		uint8_t va = 0x02;	// I2C bypass mode aka Pass-Through
		vu = I2C_transaction_write_N_regs( MPU9250_ADDRESS, INT_PIN_CFG, 1, &va );
		CDC_printf("writen %d bytes to reg INT_PIN_CFG (%02x)\n", vu, va );
		vu = I2C_transaction_read_N_regs( AK8963_ADDRESS, AK8963_WHO_AM_I, 1, &va );
		CDC_printf("read %d bytes from reg AK8963_WHO_AM_I (%02x)\n", vu, va );
		} break;

	#endif
	default:	// simple echo
		CDC_printf( "cmd '%c'\n", ((c>=' ')?(c):('?')) );
	}
}
#endif


int main(void)
{
// HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
__NVIC_SetPriorityGrouping(3);	// group 4 est represente par 3 (4 bits for pre-emption priority, 0 bits for subpriority)

// Configure the system clock
SystemClock_Config_USB();
SystemCoreClockUpdate();

gpio_init();
cntblinks = 1;

systick_init( 100 );		// 100 Hz, priorite 7

#ifdef USE_CDC
// config UART (interrupt handler doit etre pret!!)
gpio_uart2_init();
UART2_init( 38400 );
CDC_init();	// on doit faire cela avant d'entrer dans la main loop,
CDC.verbose = 1;
CDC_printf("Hello je suis imposant\n");
#endif

#ifdef ENCODER_TIM
gpio_encoder_t1_init();
encoder_init( TIM1 );
#endif

#ifdef USE_ADC_4CH
gpio_adc4_init();
adc_init();
adc_calib();
adc_timer_init( SystemCoreClock / ADCFREQ );
#endif

#ifdef USE_I2C
gpio_sw_i2c_init();
#endif

#ifdef MIDI_USB
if	(USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS) != USBD_OK)
	 while(1) {};
if	(USBD_RegisterClass(&hUsbDeviceFS, &USBD_HID) != USBD_OK)
	while(1) {};
if	(USBD_Start(&hUsbDeviceFS) != USBD_OK)
	while(1) {};
sys_delay(100);
cmd_handler( 's' );
USB_DP_pullup( 1 );
sys_delay(100);
cmd_handler( 's' );
sys_delay(100);
cmd_handler( 's' );
#endif

// // LA GROSSE BOUCLE MAIN LOOP
int c;
while (1)
  	{
 	static unsigned int old1Hz = 0;

	// actions immediates
	if	( ( c = CDC_getcmd() ) > 0 )
		cmd_handler( c );
	#ifdef MIDI_USB
	if	( ( BLUE_PRESS() ) && ( cntblinks < 5 ) )
		{
		while( ((USBD_HID_HandleTypeDef *) hUsbDeviceFS.pClassData)->state == HID_BUSY ) { CDC_printf("/"); }
		USBD_HID_SendReport(&hUsbDeviceFS, midiAllOff, 4);
		cntblinks = 5;
		}
	#endif

	#ifdef USE_ADC_4CH
	if	( adc_res_ready == 2 )
		{
		const uint32_t mVref = 1200;
		uint32_t ref = adc1_res1;
		uint32_t x = ( mVref * adc2_res0 ) / ref;
		uint32_t y = ( mVref * adc1_res0 ) / ref;
		uint32_t v = ( mVref * 4095 * CHANFIR ) / ref;
		CDC_printf( "adc X:Y = %4u : %4u mV, Vdd = %4u mV\n", x, y, v );
		// CDC_printf( "adc %u %u %u\n", adc1_res0/CHANFIR, adc2_res0/CHANFIR, adc1_res1/CHANFIR );
		adc_res_ready = 0;
		}
	#endif

	// actions 1 fois par seconde
 	if	(  ( old1Hz != cnt1Hz ) )
 		{
 		old1Hz = cnt1Hz;
 		CDC_printf( "%u s DWT test %u\n", cnt1Hz, DWT->CYCCNT / SystemCoreClock );

	#ifdef ENCODER_TIM
		short int e = encoder_get(TIM1);
		//CDC_printf( "b=%d, xy=%d:%d, e=%u\n", LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_13 ),
		//	LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_8 ), LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_9 ), e );
		// bornage pour MIDI
		e /= 4;	// cheap encoder in mode X4
		midiNoteOn[2] = midiNoteOffOn[6] = 65 + ( (e+100000000) % 13 );		// encoder -> gamme chromatique sur 1 octave a partir de Fa
	#endif

	#ifdef MIDI_USB
		if	( hUsbDeviceFS.dev_state == 3 )
			{
		#ifdef DOUBLE_EVENT
			while( ((USBD_HID_HandleTypeDef *) hUsbDeviceFS.pClassData)->state == HID_BUSY ) { /* CDC_printf(".");*/ }
			USBD_HID_SendReport(&hUsbDeviceFS, midiNoteOffOn, 8);	// note off etait prepare lors du note on precedent ;-)
			midiNoteOffOn[2] = midiNoteOffOn[6];				// pret pour le prochain tour
		#else
			unsigned int finedelay = ( 2 * 72000 ) / 3;	// 72000 = 1ms (ne pas depasser 4ms)
			// Make sure the USB functions are not BUSY before sending the MIDI Message
			while( ((USBD_HID_HandleTypeDef *) hUsbDeviceFS.pClassData)->state == HID_BUSY ) { CDC_printf("."); }
			USBD_HID_SendReport(&hUsbDeviceFS, midiNoteOff, 4);	// note off etait prepare lors du note on precedent ;-)
			midiNoteOff[2] = midiNoteOn[2];				// pret pour le prochain tour
			tickdelay( finedelay );
			while( ((USBD_HID_HandleTypeDef *) hUsbDeviceFS.pClassData)->state == HID_BUSY ) { CDC_printf(":"); }
			USBD_HID_SendReport(&hUsbDeviceFS, midiNoteOn, 4);
		#endif
			}
		if	( cntblinks == 5 )
			cntblinks = 1;		// re-armer BLUE_PRESS
	#endif
		}
	}
}

