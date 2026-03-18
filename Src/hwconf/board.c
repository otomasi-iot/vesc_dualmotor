/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "hal.h"
#include "hw.h"

#if HAL_USE_PAL || defined(__DOXYGEN__)
/**
 * @brief   PAL setup.
 * @details Digital I/O ports static configuration as defined in @p board.h.
 *          This variable is used by the HAL when initializing the PAL driver.
 */
const PALConfig pal_default_config = {
#if STM32_HAS_GPIOA
  {VAL_GPIOA_MODER, VAL_GPIOA_OTYPER, VAL_GPIOA_OSPEEDR, VAL_GPIOA_PUPDR,
   VAL_GPIOA_ODR,   VAL_GPIOA_AFRL,   VAL_GPIOA_AFRH},
#endif
#if STM32_HAS_GPIOB
  {VAL_GPIOB_MODER, VAL_GPIOB_OTYPER, VAL_GPIOB_OSPEEDR, VAL_GPIOB_PUPDR,
   VAL_GPIOB_ODR,   VAL_GPIOB_AFRL,   VAL_GPIOB_AFRH},
#endif
#if STM32_HAS_GPIOC
  {VAL_GPIOC_MODER, VAL_GPIOC_OTYPER, VAL_GPIOC_OSPEEDR, VAL_GPIOC_PUPDR,
   VAL_GPIOC_ODR,   VAL_GPIOC_AFRL,   VAL_GPIOC_AFRH},
#endif
#if STM32_HAS_GPIOD
  {VAL_GPIOD_MODER, VAL_GPIOD_OTYPER, VAL_GPIOD_OSPEEDR, VAL_GPIOD_PUPDR,
   VAL_GPIOD_ODR,   VAL_GPIOD_AFRL,   VAL_GPIOD_AFRH},
#endif
#if STM32_HAS_GPIOE
  {VAL_GPIOE_MODER, VAL_GPIOE_OTYPER, VAL_GPIOE_OSPEEDR, VAL_GPIOE_PUPDR,
   VAL_GPIOE_ODR,   VAL_GPIOE_AFRL,   VAL_GPIOE_AFRH},
#endif
#if STM32_HAS_GPIOF
  {VAL_GPIOF_MODER, VAL_GPIOF_OTYPER, VAL_GPIOF_OSPEEDR, VAL_GPIOF_PUPDR,
   VAL_GPIOF_ODR,   VAL_GPIOF_AFRL,   VAL_GPIOF_AFRH},
#endif
#if STM32_HAS_GPIOG
  {VAL_GPIOG_MODER, VAL_GPIOG_OTYPER, VAL_GPIOG_OSPEEDR, VAL_GPIOG_PUPDR,
   VAL_GPIOG_ODR,   VAL_GPIOG_AFRL,   VAL_GPIOG_AFRH},
#endif
#if STM32_HAS_GPIOH
  {VAL_GPIOH_MODER, VAL_GPIOH_OTYPER, VAL_GPIOH_OSPEEDR, VAL_GPIOH_PUPDR,
   VAL_GPIOH_ODR,   VAL_GPIOH_AFRL,   VAL_GPIOH_AFRH},
#endif
#if STM32_HAS_GPIOI
  {VAL_GPIOI_MODER, VAL_GPIOI_OTYPER, VAL_GPIOI_OSPEEDR, VAL_GPIOI_PUPDR,
   VAL_GPIOI_ODR,   VAL_GPIOI_AFRL,   VAL_GPIOI_AFRH}
#endif
};
#endif

/**
 * @brief   Early initialization code.
 * @details This initialization must be performed just after stack setup
 *          and before any other initialization.
 */
void __early_init(void) {
  HW_VERY_EARLY_INIT();
  stm32_clock_init();
}

#if HAL_USE_SDC || defined(__DOXYGEN__)
/**
 * @brief   SDC card detection.
 */
bool sdc_lld_is_card_inserted(SDCDriver *sdcp) {

  (void)sdcp;
  /* TODO: Fill the implementation.*/
  return true;
}

/**
 * @brief   SDC card write protection detection.
 */
bool sdc_lld_is_write_protected(SDCDriver *sdcp) {

  (void)sdcp;
  /* TODO: Fill the implementation.*/
  return false;
}
#endif /* HAL_USE_SDC */

#if HAL_USE_MMC_SPI || defined(__DOXYGEN__)
/**
 * @brief   MMC_SPI card detection.
 */
bool mmc_lld_is_card_inserted(MMCDriver *mmcp) {

  (void)mmcp;
  /* TODO: Fill the implementation.*/
  return true;
}

/**
 * @brief   MMC_SPI card write protection detection.
 */
bool mmc_lld_is_write_protected(MMCDriver *mmcp) {

  (void)mmcp;
  /* TODO: Fill the implementation.*/
  return false;
}
#endif

/**
 * @brief   Board-specific initialization code.
 * @todo    Add your board-specific code, if any.
 */
void boardInit(void) {
}

#include "hwconf/hal_gpio.h"
#include "hwconf/pins.h"

void hw_init_gpio(void) {
    // Enable ALL GPIO clocks
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
  
    // ===== MOTOR 1 GATE DRIVERS =====
    hal_gpio_init_output(GATE_M1_UH_PORT, GATE_M1_UH_PIN);
    hal_gpio_init_output(GATE_M1_UL_PORT, GATE_M1_UL_PIN);
    hal_gpio_init_output(GATE_M1_VH_PORT, GATE_M1_VH_PIN);
    hal_gpio_init_output(GATE_M1_VL_PORT, GATE_M1_VL_PIN);
    hal_gpio_init_output(GATE_M1_WH_PORT, GATE_M1_WH_PIN);
    hal_gpio_init_output(GATE_M1_WL_PORT, GATE_M1_WL_PIN);
  
    // ===== MOTOR 2 GATE DRIVERS =====
    hal_gpio_init_output(GATE_M2_UH_PORT, GATE_M2_UH_PIN);
    hal_gpio_init_output(GATE_M2_UL_PORT, GATE_M2_UL_PIN);
    hal_gpio_init_output(GATE_M2_VH_PORT, GATE_M2_VH_PIN);
    hal_gpio_init_output(GATE_M2_VL_PORT, GATE_M2_VL_PIN);
    hal_gpio_init_output(GATE_M2_WH_PORT, GATE_M2_WH_PIN);
    hal_gpio_init_output(GATE_M2_WL_PORT, GATE_M2_WL_PIN);
  
    // ===== FAULT INPUTS =====
    hal_gpio_init_input(FAULT_M1_PORT, FAULT_M1_PIN);
    hal_gpio_init_input(FAULT_M2_PORT, FAULT_M2_PIN);
  
    // ===== LEDs =====
    hal_gpio_init_output(LED_PORT, LED_PIN);
  
    // ===== HALL SENSORS =====
    hal_gpio_init_input_pullup(HALL_M1_A_PORT, HALL_M1_A_PIN);
    hal_gpio_init_input_pullup(HALL_M1_B_PORT, HALL_M1_B_PIN);
    hal_gpio_init_input_pullup(HALL_M1_C_PORT, HALL_M1_C_PIN);
  
    hal_gpio_init_input_pullup(HALL_M2_A_PORT, HALL_M2_A_PIN);
    hal_gpio_init_input_pullup(HALL_M2_B_PORT, HALL_M2_B_PIN);
    hal_gpio_init_input_pullup(HALL_M2_C_PORT, HALL_M2_C_PIN);
  
    // ===== I2C BIT-BANG PINS =====
    hal_gpio_init_input_pullup(I2C_SCL_PORT, I2C_SCL_PIN);
    hal_gpio_init_input_pullup(I2C_SDA_PORT, I2C_SDA_PIN);
  
    // Set initial gate states to LOW (safe)
    hal_gpio_write(GATE_M1_UH_PORT, GATE_M1_UH_PIN, 0);
    hal_gpio_write(GATE_M1_UL_PORT, GATE_M1_UL_PIN, 0);
    hal_gpio_write(GATE_M1_VH_PORT, GATE_M1_VH_PIN, 0);
    hal_gpio_write(GATE_M1_VL_PORT, GATE_M1_VL_PIN, 0);
    hal_gpio_write(GATE_M1_WH_PORT, GATE_M1_WH_PIN, 0);
    hal_gpio_write(GATE_M1_WL_PORT, GATE_M1_WL_PIN, 0);
  
    hal_gpio_write(GATE_M2_UH_PORT, GATE_M2_UH_PIN, 0);
    hal_gpio_write(GATE_M2_UL_PORT, GATE_M2_UL_PIN, 0);
    hal_gpio_write(GATE_M2_VH_PORT, GATE_M2_VH_PIN, 0);
    hal_gpio_write(GATE_M2_VL_PORT, GATE_M2_VL_PIN, 0);
    hal_gpio_write(GATE_M2_WH_PORT, GATE_M2_WH_PIN, 0);
    hal_gpio_write(GATE_M2_WL_PORT, GATE_M2_WL_PIN, 0);
}

/**
 * Dual-Motor Hardware Synchronization Verification
 * 
 * Confirms TIM1 (M1) and TIM8 (M2) are phase-locked with 180° offset.
 * Call this after mcpwm_init_hardware() completes to validate sync.
 * 
 * Expected values:
 * - TIM1->ARR = TIM8->ARR = 4499 (16 kHz @ 72 MHz)
 * - TIM8->CNT should lag TIM1->CNT by ~2250 counts (180° phase)
 * - Phase tolerance: ±100 counts (~1.4° or 19 µs)
 */
void hw_verify_dual_motor_sync_init(void) {
    #ifdef HW_HAS_DUAL_MOTORS
    extern void hw_verify_dual_motor_sync(void);
    hw_verify_dual_motor_sync();
    #endif
}
