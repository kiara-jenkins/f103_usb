
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
#include "CDC.h"

// static data
USBD_HandleTypeDef hUsbDeviceFS;
extern PCD_HandleTypeDef hpcd_USB_FS;

// MIDI Message arrays: 4 Bytes
// 1st byte : MSBs --> Cable Number 0, LSBs --> duplicate MSBs of MIDI opcode (!??!)
uint8_t midiNoteOn[4] = { 0x09, 0x93, 65, 99 };
uint8_t midiNoteOff[4]= { 0x08, 0x83, 65, 99 };


// USB interrupt routine
void USB_LP_CAN1_RX0_IRQHandler(void)
{
HAL_PCD_IRQHandler(&hpcd_USB_FS);
}

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
systick_init( 100 );		// 100 Hz, priorite 7

gpio_init();

#ifdef USE_CDC
// config UART (interrupt handler doit etre pret!!)
gpio_uart2_init();
UART2_init( 38400 );
CDC_init();	// on doit faire cela avant d'entrer dans la main loop,
CDC.verbose = 1;
CDC_printf("Hello je suis imposant\n");
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


USB_DP_pullup( 1 );
sys_delay(500);

// main loop
int c;
while (1)
  	{
	// Make sure the USB functions are not BUSY before sending the MIDI Message
	while( ((USBD_HID_HandleTypeDef *) hUsbDeviceFS.pClassData)->state == HID_BUSY ) {}
	USBD_HID_SendReport(&hUsbDeviceFS, midiNoteOn, 4); LED_ON();
	sys_delay(50);
	if	( ( c = CDC_getcmd() ) > 0 )
		cmd_handler( c );

	while( ((USBD_HID_HandleTypeDef *) hUsbDeviceFS.pClassData)->state == HID_BUSY ) {}
	USBD_HID_SendReport(&hUsbDeviceFS, midiNoteOff, 4); LED_OFF();
	sys_delay(50);
	if	( ( c = CDC_getcmd() ) > 0 )
		cmd_handler( c );

	}
}

