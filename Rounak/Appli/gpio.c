#include "options.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_gpio.h"
// #include "sys.h"
#include "gpio.h"

// N.B. LL_GPIO_MODE_FLOATING <==> pas de pull
//      LL_GPIO_MODE_INPUT    <==> pull up ou down selon ODR

void gpio_init(void)
{
// ex MSP support aka MCU Configuration
//  __HAL_RCC_AFIO_CLK_ENABLE();
//  __HAL_RCC_PWR_CLK_ENABLE();
//  __HAL_AFIO_REMAP_SWJ_NOJTAG();
LL_APB2_GRP1_EnableClock( LL_APB2_GRP1_PERIPH_AFIO );
LL_APB1_GRP1_EnableClock( LL_APB1_GRP1_PERIPH_PWR );
LL_GPIO_AF_Remap_SWJ_NOJTAG();	// NOJTAG: JTAG-DP Disabled and SW-DP Enabled

#ifdef NUCLEO
// Nucleo LED = PA5 act. hi	!!! ecrase par SPI SCK
LL_APB2_GRP1_EnableClock( LL_APB2_GRP1_PERIPH_GPIOA );
LL_GPIO_SetPinMode(       GPIOA, LL_GPIO_PIN_5, LL_GPIO_MODE_OUTPUT );
LL_GPIO_SetPinOutputType( GPIOA, LL_GPIO_PIN_5, LL_GPIO_OUTPUT_PUSHPULL );
// blue button act. lo
LL_APB2_GRP1_EnableClock( LL_APB2_GRP1_PERIPH_GPIOC );
LL_GPIO_SetPinMode(       GPIOC, LL_GPIO_PIN_13, LL_GPIO_MODE_FLOATING );
#else
// blue pill LED = PC13 act. lo
LL_APB2_GRP1_EnableClock( LL_APB2_GRP1_PERIPH_GPIOC );
LL_GPIO_SetPinMode(       GPIOC, LL_GPIO_PIN_13, LL_GPIO_MODE_OUTPUT );
LL_GPIO_SetPinOutputType( GPIOC, LL_GPIO_PIN_13, LL_GPIO_OUTPUT_PUSHPULL );
#endif

// PA11 et PA12 pour USB : 
// "As soon as the USB is enabled, these pins are automatically connected to the USB internal transceiver"
// faut quand meme horloge (et aussi D ?)
LL_APB2_GRP1_EnableClock( LL_APB2_GRP1_PERIPH_GPIOA );
} // gpio_init(void)


