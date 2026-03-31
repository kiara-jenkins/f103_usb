extern volatile unsigned int cnt100Hz;
extern volatile unsigned int cnt1Hz;

#ifdef __cplusplus
extern "C" {
#endif

// systick avec interrupt
void systick_init( unsigned int freq );

// temporisation basee sur cnt100Hz
void sys_delay( uint32_t delay );

// temporisation base sur systick
// unites en periodes d'horloge du timer ( HCLK ou HCLK/8 )
// tickd doit etre inferieur a (LOAD+1)/2
void tickdelay( unsigned int tickd );

// creer un report dans le buffer
// N.B. pour avoir la correspondance numero <--> perif , voir IRQn_Type
// F103 : UARTS 1,2,3 : 37, 38, 39; TIM 2, 3, 4 : 28, 29, 30
void report_interrupts();

// disable systick interrupt
void systick_no_interrupt();

// System Clock Configuration selon options
void SystemClock_Config_USB(void);

// temporisation base sur DWT cycle counter (cf core_cm3.h et ARMv7-M_ARM.pdf p. C1-49)
// unites en periodes d'horloge CPU
__STATIC_INLINE void DWT_start()
{
  CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk;
  CoreDebug->DEMCR |=  CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |=  DWT_CTRL_CYCCNTENA_Msk;
  DWT->CYCCNT = 0;
}

__STATIC_INLINE void DWT_stop()
{
  CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk;
  CoreDebug->DEMCR |=  CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk;
  DWT->CYCCNT = 0;
}

__STATIC_INLINE int DWT_is_running()
{
  uint32_t now = DWT->CYCCNT;
  // verifier qu'il tourne !
  __ASM volatile ("NOP");
  __ASM volatile ("NOP");
  __ASM volatile ("NOP");
  return ( DWT->CYCCNT != now );
}

__STATIC_INLINE void DWT_delay(uint32_t cycles)
{
uint32_t clk_cycle_start = DWT->CYCCNT;
while	( ( DWT->CYCCNT - clk_cycle_start ) < cycles )
	{};
}

#ifdef __cplusplus
}
#endif
