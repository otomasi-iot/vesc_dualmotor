/*
    Copyright 2016 Benjamin Vedder  benjamin@vedder.se

    This file is part of the VESC firmware.
*/

#include "stm32f1xx_hal.h"
#include "stm32f1xx_ll_tim.h"
#include "mc_interface.h"
#include "mcpwm_foc.h"
#include "hw.h"
#include "encoder/encoder.h"
#include "main.h"

// Forward declare HAL timer and ADC handles
extern TIM_HandleTypeDef htim1, htim2, htim8;
extern ADC_HandleTypeDef hadc1;

// ISR dispatcher for TIM1 (Update, Compare)
void TIM1_UP_IRQHandler(void) {
    HAL_TIM_IRQHandler(&htim1);
}

void TIM1_CC_IRQHandler(void) {
    HAL_TIM_IRQHandler(&htim1);
}

// ISR dispatcher for TIM2
void TIM2_IRQHandler(void) {
    HAL_TIM_IRQHandler(&htim2);
}

// ISR dispatcher for TIM8
void TIM8_UP_IRQHandler(void) {
    HAL_TIM_IRQHandler(&htim8);
}

void TIM8_CC_IRQHandler(void) {
    HAL_TIM_IRQHandler(&htim8);
}

// ISR dispatcher for ADC1
void ADC1_2_IRQHandler(void) {
    HAL_ADC_IRQHandler(&hadc1);
}

// DMA1 Channel1 ISR for ADC - Motor FOC current sampling ISR
// Called at 6 kHz (ADC sample rate) or as configured by TIM2 CC2 trigger
extern DMA_HandleTypeDef hdma_adc;
extern void mcpwm_foc_adc_int_handler(void *p, uint32_t flags);

void DMA1_Channel1_IRQHandler(void) {
    // Call HAL DMA handler which will trigger DMA callbacks
    HAL_DMA_IRQHandler(&hdma_adc);
}

// DMA transfer complete callback (called by HAL_DMA_IRQHandler)
void HAL_DMA_XferCpltCallback(DMA_HandleTypeDef *hdma) {
    if (hdma->Instance == DMA1_Channel1) {
        // Call motor FOC ISR (critical path for PWM duty update)
        mcpwm_foc_adc_int_handler(NULL, 0);
    }
}

// DMA transfer half-complete callback
void HAL_DMA_XferHalfCpltCallback(DMA_HandleTypeDef *hdma) {
    if (hdma->Instance == DMA1_Channel1) {
        // Also trigger on half-complete for low-latency dual-motor sampling
        mcpwm_foc_adc_int_handler(NULL, 0);
    }
}

// Fault/DCDC ISR
void EXTI_OCP_IRQHandler(void) {
    // Handle OCP (over-current protection)
    // HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_X);  // Fault input pin
}

// Encoder EXTI interrupt
void HW_ENC_EXTI_ISR_VEC(void) {
    if (__HAL_GPIO_EXTI_GET_IT(HW_ENC_EXTI_LINE) != RESET) {
        encoder_pin_isr();
        __HAL_GPIO_EXTI_CLEAR_IT(HW_ENC_EXTI_LINE);
    }
}

// Encoder timer overflow interrupt
void HW_ENC_TIM_ISR_VEC(void) {
    if (LL_TIM_IsActiveFlag_UPDATE(HW_ENC_TIM)) {
        encoder_tim_isr();
        LL_TIM_ClearFlag_UPDATE(HW_ENC_TIM);
    }
}

// Power voltage detector - under-voltage fault
void PVD_IRQHandler(void) {
    if (__HAL_GPIO_EXTI_GET_IT(EXTI_LINE_16) != RESET) {
        mc_interface_fault_stop(FAULT_CODE_MCU_UNDER_VOLTAGE, false, true);
        __HAL_GPIO_EXTI_CLEAR_IT(EXTI_LINE_16);
    }
}

// Fault handlers
void NMI_Handler(void) {
    main_stop_motor_and_reset();
}

void HardFault_Handler(void) {
    main_stop_motor_and_reset();
}
