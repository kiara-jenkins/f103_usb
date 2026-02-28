
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
#include "CDC.h"

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


// USB interrupt routine
void USB_LP_CAN1_RX0_IRQHandler(void)
{
HAL_PCD_IRQHandler(&hpcd_USB_FS);
}

#ifdef __cplusplus
}
#endif

#ifdef USE_CDC
void cmd_handler( int c )
{
switch	( c )
	{
	case '&' :
		CDC_printf( "HCLK %u\n", SystemCoreClock );
		break;
	case '$' :
		report_interrupts();
		break;
	case 'e' :
		CDC_printf( "b=%d, xy=%d:%d, e=%u\n", LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_13 ),
			LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_6 ), LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_7 ), encoder_get(TIM3) );
		break;
	case 's' :
		/* vu dans usbd_def.h
		#define USBD_STATE_DEFAULT                              0x01U
    		#define USBD_STATE_ADDRESSED                            0x02U
		#define USBD_STATE_CONFIGURED                           0x03U
		#define USBD_STATE_SUSPENDED                            0x04U
		c'est hUsbDeviceFS.dev_state !! eureka !!
		*/
		CDC_printf( "status %u %u %u %u %u\n",
			hUsbDeviceFS.dev_config_status,
			hUsbDeviceFS.dev_state,
			hUsbDeviceFS.dev_connection_status,
			hUsbDeviceFS.ep_out[1].status,
			((USBD_HID_HandleTypeDef *) hUsbDeviceFS.pClassData)->state
			);
		break;
	case '0' :
	case '1' :
	case '2' :
	case '3' :
	case '4' :
	case '5' : cntblinks = c - '0';
		break;
	default:	// simple echo
		CDC_printf( "cmd '%c'\n", ((c>=' ')?(c):('?')) );
	}
}
#endif


int main(void)
{
// HAL_Init() flattened to 4 statements :
// __HAL_FLASH_PREFETCH_BUFFER_ENABLE();	// enabled by default
// HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
__NVIC_SetPriorityGrouping(3);	// group 4 est represente par 3 (4 bits for pre-emption priority, 0 bits for subpriority)
// HAL_InitTick(TICK_INT_PRIORITY);	// ridicule, il devra le refaire apres avoir configure la PLL
// HAL_MspInit();			// toxic weak function, but 3 LL-translated lines were put in gpio.c

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
gpio_encoder_t3_init();
encoder_init( TIM3 );
#endif


  /* Init Device Library, add supported class and start the library. */
  if (USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS) != USBD_OK)
  {
    while(1) {}
  }
  if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_HID) != USBD_OK)
  {
    while(1) {}
  }
  if (USBD_Start(&hUsbDeviceFS) != USBD_OK)
  {
    while(1) {}
  }

sys_delay(100);
cmd_handler( 's' );

USB_DP_pullup( 1 );
sys_delay(100);
cmd_handler( 's' );
sys_delay(100);
cmd_handler( 's' );

// // LA GROSSE BOUCLE MAIN LOOP
int c;
while (1)
  	{
 	static unsigned int old1Hz = 0;

	// actions immediates
	if	( ( c = CDC_getcmd() ) > 0 )
		cmd_handler( c );
	if	( ( BLUE_PRESS() ) && ( cntblinks < 5 ) )
		{
		while( ((USBD_HID_HandleTypeDef *) hUsbDeviceFS.pClassData)->state == HID_BUSY ) { CDC_printf("/"); }
		USBD_HID_SendReport(&hUsbDeviceFS, midiAllOff, 4);
		cntblinks = 5;
		}

	// actions 1 fois par seconde
 	if	(  ( old1Hz != cnt1Hz ) )
 		{
 		old1Hz = cnt1Hz;

		short int e = encoder_get(TIM3);
		CDC_printf( "b=%d, xy=%d:%d, e=%d\n", LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_13 ),
			LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_6 ), LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_7 ), (int)e );
		if	( e < 0 ) e = 0;
		if	( e > 24 ) e = 24;
		e /= 2;	// cheap encoder in mode X2

		#ifdef DOUBLE_EVENT
		midiNoteOffOn[6] = 65 + e;		// gamme chromatique sur 1 octave a partir de Fa
		while( ((USBD_HID_HandleTypeDef *) hUsbDeviceFS.pClassData)->state == HID_BUSY ) { /* CDC_printf(".");*/ }
		USBD_HID_SendReport(&hUsbDeviceFS, midiNoteOffOn, 8);	// note off etait prepare lors du note on precedent ;-)
		midiNoteOffOn[2] = midiNoteOffOn[6];				// pret pour le prochain tour
		#else
		unsigned int finedelay = ( 2 * 72000 ) / 3;	// 72000 = 1ms (ne pas depasser 4ms)
		midiNoteOn[2] = 65 + e;		// gamme chromatique sur 1 octave a partir de Fa
		// Make sure the USB functions are not BUSY before sending the MIDI Message
		while( ((USBD_HID_HandleTypeDef *) hUsbDeviceFS.pClassData)->state == HID_BUSY ) { /* CDC_printf(".");*/ }
		USBD_HID_SendReport(&hUsbDeviceFS, midiNoteOff, 4);	// note off etait prepare lors du note on precedent ;-)
		midiNoteOff[2] = midiNoteOn[2];				// pret pour le prochain tour
		tickdelay( finedelay );
		while( ((USBD_HID_HandleTypeDef *) hUsbDeviceFS.pClassData)->state == HID_BUSY ) { /*CDC_printf(":");*/ }
		USBD_HID_SendReport(&hUsbDeviceFS, midiNoteOn, 4);
		#endif

		if	( cntblinks == 5 )
			cntblinks = 1;		// re-armer BLUE_PRESS
		}
	}
}

