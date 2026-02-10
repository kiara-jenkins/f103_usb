/**
  ******************************************************************************
  * @file    stm32f1xx_hal.c
  * @author  MCD Application Team
  * @brief   HAL module driver.
  *          This is the common part of the HAL initialization
  *
  @verbatim
  ==============================================================================
                     ##### How to use this driver #####
  ==============================================================================
    [..]
    The common HAL driver contains a set of generic and common APIs that can be
    used by the PPP peripheral drivers and the user to start using the HAL.
    [..]
    The HAL contains two APIs' categories:
         (+) Common HAL APIs
         (+) Services HAL APIs

  @endverbatim
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/** @addtogroup STM32F1xx_HAL_Driver
  * @{
  */

/** @defgroup HAL HAL
  * @brief HAL module driver.
  * @{
  */

#ifdef HAL_MODULE_ENABLED

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/** @defgroup HAL_Private_Constants HAL Private Constants
  * @{
  */
/**
 * @brief STM32F1xx HAL Driver version number V1.1.8
   */
#define __STM32F1xx_HAL_VERSION_MAIN   (0x01U) /*!< [31:24] main version */
#define __STM32F1xx_HAL_VERSION_SUB1   (0x01U) /*!< [23:16] sub1 version */
#define __STM32F1xx_HAL_VERSION_SUB2   (0x08U) /*!< [15:8]  sub2 version */
#define __STM32F1xx_HAL_VERSION_RC     (0x00U) /*!< [7:0]  release candidate */
#define __STM32F1xx_HAL_VERSION         ((__STM32F1xx_HAL_VERSION_MAIN << 24)\
                                        |(__STM32F1xx_HAL_VERSION_SUB1 << 16)\
                                        |(__STM32F1xx_HAL_VERSION_SUB2 << 8 )\
                                        |(__STM32F1xx_HAL_VERSION_RC))

#define IDCODE_DEVID_MASK    0x00000FFFU

/**
  * @}
  */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/** @defgroup HAL_Private_Variables HAL Private Variables
  * @{
  */
__IO uint32_t uwTick;
__weak void HAL_IncTick(void)
{
  uwTick += 1;
}

/**
  * @brief Provides a tick value in millisecond.
  * @note  This function is declared as __weak to be overwritten in case of other
  *       implementations in user file.
  * @retval tick value
  */
__weak uint32_t HAL_GetTick(void)
{
  return uwTick;
}

/**
  * @brief This function returns a tick priority.
  * @retval tick priority
  */
uint32_t HAL_GetTickPrio(void)
{
  return uwTickPrio;
}

/**
  * @brief Set new tick Freq.
  * @retval status
  */
HAL_StatusTypeDef HAL_SetTickFreq(HAL_TickFreqTypeDef Freq)
{
  HAL_StatusTypeDef status  = HAL_OK;
  HAL_TickFreqTypeDef prevTickFreq;


  if (uwTickFreq != Freq)
  {
    /* Back up uwTickFreq frequency */
    prevTickFreq = uwTickFreq;

    /* Update uwTickFreq global variable used by HAL_InitTick() */
    uwTickFreq = Freq;

    /* Apply the new tick Freq  */
    status = HAL_InitTick(uwTickPrio);

    if (status != HAL_OK)
    {
      /* Restore previous tick frequency */
      uwTickFreq = prevTickFreq;
    }
  }

  return status;
}

/**
  * @brief Return tick frequency.
  * @retval tick period in Hz
  */
HAL_TickFreqTypeDef HAL_GetTickFreq(void)
{
  return uwTickFreq;
}

/**
  * @brief This function provides minimum delay (in milliseconds) based
  *        on variable incremented.
  * @note In the default implementation , SysTick timer is the source of time base.
  *       It is used to generate interrupts at regular time intervals where uwTick
  *       is incremented.
  * @note This function is declared as __weak to be overwritten in case of other
  *       implementations in user file.
  * @param Delay specifies the delay time length, in milliseconds.
  * @retval None
  */
__weak void HAL_Delay(uint32_t Delay)
{
  uint32_t tickstart = HAL_GetTick();
  uint32_t wait = Delay;

  /* Add a freq to guarantee minimum wait */
  if (wait < HAL_MAX_DELAY)
  {
    wait += 1;
  }

  while ((HAL_GetTick() - tickstart) < wait)
  {
  }
}

/**
  * @brief Suspend Tick increment.
  * @note In the default implementation , SysTick timer is the source of time base. It is
  *       used to generate interrupts at regular time intervals. Once HAL_SuspendTick()
  *       is called, the SysTick interrupt will be disabled and so Tick increment
  *       is suspended.
  * @note This function is declared as __weak to be overwritten in case of other
  *       implementations in user file.
  * @retval None
  */
__weak void HAL_SuspendTick(void)
{
  /* Disable SysTick Interrupt */
  CLEAR_BIT(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk);
}

/**
  * @brief Resume Tick increment.
  * @note In the default implementation , SysTick timer is the source of time base. It is
  *       used to generate interrupts at regular time intervals. Once HAL_ResumeTick()
  *       is called, the SysTick interrupt will be enabled and so Tick increment
  *       is resumed.
  * @note This function is declared as __weak to be overwritten in case of other
  *       implementations in user file.
  * @retval None
  */
__weak void HAL_ResumeTick(void)
{
  /* Enable SysTick Interrupt */
  SET_BIT(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk);
}

/**
  * @brief  Returns the HAL revision
  * @retval version 0xXYZR (8bits for each decimal, R for RC)
  */
uint32_t HAL_GetHalVersion(void)
{
  return __STM32F1xx_HAL_VERSION;
}

/**
  * @brief Returns the device revision identifier.
  * Note: On devices STM32F10xx8 and STM32F10xxB,
  *                  STM32F101xC/D/E and STM32F103xC/D/E,
  *                  STM32F101xF/G and STM32F103xF/G
  *                  STM32F10xx4 and STM32F10xx6
  *       Debug registers DBGMCU_IDCODE and DBGMCU_CR are accessible only in
  *       debug mode (not accessible by the user software in normal mode).
  *       Refer to errata sheet of these devices for more details.
  * @retval Device revision identifier
  */
uint32_t HAL_GetREVID(void)
{
  return ((DBGMCU->IDCODE) >> DBGMCU_IDCODE_REV_ID_Pos);
}

/**
  * @brief  Returns the device identifier.
  * Note: On devices STM32F10xx8 and STM32F10xxB,
  *                  STM32F101xC/D/E and STM32F103xC/D/E,
  *                  STM32F101xF/G and STM32F103xF/G
  *                  STM32F10xx4 and STM32F10xx6
  *       Debug registers DBGMCU_IDCODE and DBGMCU_CR are accessible only in
  *       debug mode (not accessible by the user software in normal mode).
  *       Refer to errata sheet of these devices for more details.
  * @retval Device identifier
  */
uint32_t HAL_GetDEVID(void)
{
  return ((DBGMCU->IDCODE) & IDCODE_DEVID_MASK);
}

/**
  * @brief  Returns first word of the unique device identifier (UID based on 96 bits)
  * @retval Device identifier
  */
uint32_t HAL_GetUIDw0(void)
{
   return(READ_REG(*((uint32_t *)UID_BASE)));
}

/**
  * @brief  Returns second word of the unique device identifier (UID based on 96 bits)
  * @retval Device identifier
  */
uint32_t HAL_GetUIDw1(void)
{
   return(READ_REG(*((uint32_t *)(UID_BASE + 4U))));
}

/**
  * @brief  Returns third word of the unique device identifier (UID based on 96 bits)
  * @retval Device identifier
  */
uint32_t HAL_GetUIDw2(void)
{
   return(READ_REG(*((uint32_t *)(UID_BASE + 8U))));
}

/**
  * @brief  Enable the Debug Module during SLEEP mode
  * @retval None
  */
void HAL_DBGMCU_EnableDBGSleepMode(void)
{
  SET_BIT(DBGMCU->CR, DBGMCU_CR_DBG_SLEEP);
}

/**
  * @brief  Disable the Debug Module during SLEEP mode
  * Note: On devices STM32F10xx8 and STM32F10xxB,
  *                  STM32F101xC/D/E and STM32F103xC/D/E,
  *                  STM32F101xF/G and STM32F103xF/G
  *                  STM32F10xx4 and STM32F10xx6
  *       Debug registers DBGMCU_IDCODE and DBGMCU_CR are accessible only in
  *       debug mode (not accessible by the user software in normal mode).
  *       Refer to errata sheet of these devices for more details.
  * @retval None
  */
void HAL_DBGMCU_DisableDBGSleepMode(void)
{
  CLEAR_BIT(DBGMCU->CR, DBGMCU_CR_DBG_SLEEP);
}

/**
  * @brief  Enable the Debug Module during STOP mode
  * Note: On devices STM32F10xx8 and STM32F10xxB,
  *                  STM32F101xC/D/E and STM32F103xC/D/E,
  *                  STM32F101xF/G and STM32F103xF/G
  *                  STM32F10xx4 and STM32F10xx6
  *       Debug registers DBGMCU_IDCODE and DBGMCU_CR are accessible only in
  *       debug mode (not accessible by the user software in normal mode).
  *       Refer to errata sheet of these devices for more details.
  * Note: On all STM32F1 devices:
  *       If the system tick timer interrupt is enabled during the Stop mode
  *       debug (DBG_STOP bit set in the DBGMCU_CR register ), it will wakeup
  *       the system from Stop mode.
  *       Workaround: To debug the Stop mode, disable the system tick timer
  *       interrupt.
  *       Refer to errata sheet of these devices for more details.
  * Note: On all STM32F1 devices:
  *       If the system tick timer interrupt is enabled during the Stop mode
  *       debug (DBG_STOP bit set in the DBGMCU_CR register ), it will wakeup
  *       the system from Stop mode.
  *       Workaround: To debug the Stop mode, disable the system tick timer
  *       interrupt.
  *       Refer to errata sheet of these devices for more details.
  * @retval None
  */
void HAL_DBGMCU_EnableDBGStopMode(void)
{
  SET_BIT(DBGMCU->CR, DBGMCU_CR_DBG_STOP);
}

/**
  * @brief  Disable the Debug Module during STOP mode
  * Note: On devices STM32F10xx8 and STM32F10xxB,
  *                  STM32F101xC/D/E and STM32F103xC/D/E,
  *                  STM32F101xF/G and STM32F103xF/G
  *                  STM32F10xx4 and STM32F10xx6
  *       Debug registers DBGMCU_IDCODE and DBGMCU_CR are accessible only in
  *       debug mode (not accessible by the user software in normal mode).
  *       Refer to errata sheet of these devices for more details.
  * @retval None
  */
void HAL_DBGMCU_DisableDBGStopMode(void)
{
  CLEAR_BIT(DBGMCU->CR, DBGMCU_CR_DBG_STOP);
}

/**
  * @brief  Enable the Debug Module during STANDBY mode
  * Note: On devices STM32F10xx8 and STM32F10xxB,
  *                  STM32F101xC/D/E and STM32F103xC/D/E,
  *                  STM32F101xF/G and STM32F103xF/G
  *                  STM32F10xx4 and STM32F10xx6
  *       Debug registers DBGMCU_IDCODE and DBGMCU_CR are accessible only in
  *       debug mode (not accessible by the user software in normal mode).
  *       Refer to errata sheet of these devices for more details.
  * @retval None
  */
void HAL_DBGMCU_EnableDBGStandbyMode(void)
{
  SET_BIT(DBGMCU->CR, DBGMCU_CR_DBG_STANDBY);
}

/**
  * @brief  Disable the Debug Module during STANDBY mode
  * Note: On devices STM32F10xx8 and STM32F10xxB,
  *                  STM32F101xC/D/E and STM32F103xC/D/E,
  *                  STM32F101xF/G and STM32F103xF/G
  *                  STM32F10xx4 and STM32F10xx6
  *       Debug registers DBGMCU_IDCODE and DBGMCU_CR are accessible only in
  *       debug mode (not accessible by the user software in normal mode).
  *       Refer to errata sheet of these devices for more details.
  * @retval None
  */
void HAL_DBGMCU_DisableDBGStandbyMode(void)
{
  CLEAR_BIT(DBGMCU->CR, DBGMCU_CR_DBG_STANDBY);
}

/**
  * @}
  */

/**
  * @}
  */

#endif /* HAL_MODULE_ENABLED */
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
