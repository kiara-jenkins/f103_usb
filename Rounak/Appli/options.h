
// profilage
//#define PROF_PB12

// osc. modes :
//		HSI		HSE		HSE_EXT
// fmin		 8MHz		 8MHz		 8MHz
// fmax PLL	64MHz		72MHz		72MHz
// nucleo	Y		N		Y
// nucleo cut	Y		N		N
// blue pill	Y		Y		N
// Olimex

/* resume des differences entre Nucleo et Blue Pill :
			Flash		8MHz		LED		BUTTON		PA12
	Nucleo		128k		HSE_EXT	| HSI	PA5  act hi	PC13 act lo	-
	Blue Pill	 64k		HSE		PC13 act lo	-		pullup USB @ 5V
	Olimex				HSE
	ATTENTION au linker script, celui de la Blue Pill est Debug_STM32F103C8_FLASH.ld, il est Ok pour Nucleo mais limite la flash a 64k
 */

#define NUCLEO

#ifdef NUCLEO
  #define USE_CDC
#endif

// HSE_EXT est pour utiliser une source d'horloge 8MHz externe
// sur nucleo : MCO de la sonde ST-LINK    8MHz -> PLL -> 72 MHz
// sur blue pill et Olimex : quartz local  8MHz -> PLL -> 72 MHz
#define HSE
#ifdef NUCLEO
#define HSE_EXT
#endif

#define USE_PLL	// 64 MHz (HSI) ou 72 MHZ (HSE, HSE_EXT)

#define BLINK		// tant qu'on utilise pas SPI1
#define MIDI_USB
#define ENCODER_TIM	// interface quadratic encoder utilisant un timer
#define USE_ADC_4CH
// #define USE_I2C		// I2C master
