#include "options.h" 

// interface quadratic encoder utilisant un timer
#ifdef ENCODER_TIM
#include "stm32f1xx_ll_bus.h"	// Pour horloge
#include "stm32f1xx_ll_tim.h" 

#include "encoder.h" 

/* Encoder interface (quadrature) (c'est du slave mode)
	- utiliser obligatoirement entrees T1 et T2 i.e. TIMx_CH1 et TIMx_CH2
	- ecrire dans TIMx_SMCR (c'est cela qui est essentiel : slave mode)
		SMS=011 : quad encoder : TRGI est bypasse, IC1 et IC2 contribuent a CK et DIR de CNT
	- ecrire dans TIMx_CCMR1
		CC1S=01 (IC1 mapped on TI1)
		CC2S=01 (IC2 mapped on TI2)
	- ecrire dans TIMx_CCER
		CC1P=0 (IC1 non-inverted)
		CC2P=0 (IC2 non-inverted)
	- configurer ARR pour gerer l'overflow (N.B. ARR = periode - 1)
	- lire le comptage dans TIMx_CNT
  Ou en LL : LL_TIM_ENCODER_Init() qui appelle entre autres LL_TIM_SetEncoderMode()
*/
/* choix du timer sur F103
	- les 4 timers sont 16-bit et supportent quad encoder
	- encoder is available on CH1,2 (not CH3,4)
	- TIM1 est "advanced", on peut vouloir le reserver pour autre chose
	- pinout : eviter PA11, PA12, PA6(nucleo) reserves pour USB
			Nucleo		Blue Pill
		TIM1	PA8,  PA9	PA8,  PA9	FT, UART1
		TIM2	PA0,  PA1       PA0,  PA1	3.3V, ADC
		TIM3	PC6,  PC7       		FT	  	(full remap)
			(PB4, PB5)      (PB4, PB5)	FT, SPI1, JTAG  (part remap)
		TIM4	PB6,  PB7       PB6,  PB7	FT, UART1
*/
void encoder_init( TIM_TypeDef * Timer )
{
	
// horloge
if	( Timer == TIM1 ) LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM1);
else if	( Timer == TIM2 ) LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
else if	( Timer == TIM3 ) LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);
else			  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM4);

// version Mahout : remplit les 9 params de la struct - necessite stm32f1xx_ll_tim.c
LL_TIM_ENCODER_InitTypeDef encoder_params;
// X4 signifie 4 cnts par cycle <==> tous les fronts actionnent le compteur
// X2 signifie 2 cnts par cycle <==> les fronts d'un seul input actionnent le compteur
//encoder_params.EncoderMode    = LL_TIM_ENCODERMODE_X4_TI12;	// precision encoder
encoder_params.EncoderMode    = LL_TIM_ENCODERMODE_X2_TI1;	// cheap encoder
//encoder_params.EncoderMode    = LL_TIM_ENCODERMODE_X2_TI2;	// cheap encoder
encoder_params.IC1Polarity    = LL_TIM_IC_POLARITY_RISING;
encoder_params.IC1ActiveInput = LL_TIM_ACTIVEINPUT_DIRECTTI;
encoder_params.IC1Prescaler   = LL_TIM_ICPSC_DIV1;
encoder_params.IC1Filter      = LL_TIM_IC_FILTER_FDIV1;
encoder_params.IC2Polarity    = LL_TIM_IC_POLARITY_RISING;
encoder_params.IC2ActiveInput = LL_TIM_ACTIVEINPUT_DIRECTTI;
encoder_params.IC2Prescaler   = LL_TIM_ICPSC_DIV1;
encoder_params.IC2Filter      = LL_TIM_IC_FILTER_FDIV1;
LL_TIM_ENCODER_Init( Timer, &encoder_params );

LL_TIM_SetAutoReload( Timer, (1<<16) - 1 );	// full range
TIM3->CNT = 0;
LL_TIM_EnableCounter( Timer );
}

#endif

