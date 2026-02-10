
/* Includes ------------------------------------------------------------------*/
#include "main.h"

#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_system.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_usart.h"

#include "options.h"
#include "sys.h"
#include "gpio.h"
#include "uarts.h"
#include "CDC.h"

#include "usb_device.h"
#include "usbd_hid.h"

/* USER CODE BEGIN PV */
extern USBD_HandleTypeDef hUsbDeviceFS;

// MIDI Message arrays: 4 Bytes
// 1st byte : MSBs --> Cable Number 0, LSBs --> duplicate MSBs of MIDI opcode (!??!)
uint8_t midiNoteOn[4] = { 0x09, 0x93, 65, 99 };
uint8_t midiNoteOff[4]= { 0x08, 0x83, 65, 99 };


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

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
	default:	// simple echo
		CDC_printf( "cmd '%c'\n", ((c>=' ')?(c):('?')) );
	}
}
#endif

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  // HAL_Init() flattened to 4 statements :

	/* Configure Flash prefetch */
	// __HAL_FLASH_PREFETCH_BUFFER_ENABLE();	// enabled by default
	/* Set Interrupt Group Priority */
	// HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
	__NVIC_SetPriorityGrouping(3);	// group 4 est represente par 3 (4 bits for pre-emption priority, 0 bits for subpriority)
	/* Use systick as time base source and configure 1ms tick (default clock after Reset is HSI) */
	// HAL_InitTick(TICK_INT_PRIORITY);	// ridicule, il devra le refaire apres avoir configure la PLL
	/* Init the low level hardware */
	// HAL_MspInit();			// toxic weak function, but 3 LL-translated lines were put in gpio.c

  /* Configure the system clock */
  SystemClock_Config_LL();
  SystemCoreClockUpdate();
  systick_init( 1000 );		// priorite 7

  /* Initialize peripherals */
  gpio_init();

#ifdef USE_CDC
// config UART (interrupt handler doit etre pret!!)
gpio_uart2_init();
UART2_init( 38400 );
CDC_init();	// on doit faire cela avant d'entrer dans la main loop,
CDC.verbose = 1;
CDC_printf("Hello je suis imposant\n");
#endif

  MX_USB_DEVICE_Init();

  USB_DP_pullup( 1 );
  HAL_Delay(5000);

  /* Infinite loop */
  int c;
  while (1)
  	{
	// Make sure the USB functions are not BUSY before sending the MIDI Message
	while( ((USBD_HID_HandleTypeDef *) hUsbDeviceFS.pClassData)->state == HID_BUSY ) {}
	USBD_HID_SendReport(&hUsbDeviceFS, midiNoteOn, 4); LED_ON();
	HAL_Delay(500);
	if	( ( c = CDC_getcmd() ) > 0 )
		cmd_handler( c );

	while( ((USBD_HID_HandleTypeDef *) hUsbDeviceFS.pClassData)->state == HID_BUSY ) {}
	USBD_HID_SendReport(&hUsbDeviceFS, midiNoteOff, 4); LED_OFF();
	HAL_Delay(500);
	if	( ( c = CDC_getcmd() ) > 0 )
		cmd_handler( c );

	}
}

