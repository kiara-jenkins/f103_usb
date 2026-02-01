#ifdef NUCLEO
#define LED_ON()	LL_GPIO_SetOutputPin(   GPIOA, LL_GPIO_PIN_5 )
#define LED_OFF()	LL_GPIO_ResetOutputPin( GPIOA, LL_GPIO_PIN_5 )
#define BLUE_PRESS()	(!LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_13 ))	// K2 sur carte Keil/compatible
#define K1_PRESS()	(!LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_0 ))		// K1 sur carte Keil/compatible
#else	// Blue Pill
#define LED_OFF()	LL_GPIO_SetOutputPin(   GPIOC, LL_GPIO_PIN_13 )
#define LED_ON()	LL_GPIO_ResetOutputPin( GPIOC, LL_GPIO_PIN_13 )
#define BLUE_PRESS()	(0)
#endif


#ifdef __cplusplus
extern "C" {
#endif

void gpio_init(void);

#ifdef __cplusplus
} // extern "C"
#endif
