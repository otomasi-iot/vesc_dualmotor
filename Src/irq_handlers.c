/*
	Copyright 2016 Benjamin Vedder	benjamin@vedder.se

	This file is part of the VESC firmware.

	The VESC firmware is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The VESC firmware is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    */

#include "stm32f1xx_hal.h"
#include "stm32f1xx_ll_tim.h"
#include "mc_interface.h"
#include "mcpwm_foc.h"
#include "hw.h"
#include "encoder/encoder.h"
#include "main.h"

// External DMA handle from mcpwm_foc.c
extern DMA_HandleTypeDef hdma_adc;

// DMA1 Channel1 IRQ: ADC DMA transfer complete/half-complete
// This is the primary FOC ISR trigger (replaces DMA2_Stream4 on F4)
void DMA1_Channel1_IRQHandler(void) {
	// Check half-transfer flag
	if (__HAL_DMA_GET_FLAG(&hdma_adc, DMA_FLAG_HT1)) {
		__HAL_DMA_CLEAR_FLAG(&hdma_adc, DMA_FLAG_HT1);
		mcpwm_foc_adc_int_handler(NULL, 0);
	}

	// Check transfer-complete flag
	if (__HAL_DMA_GET_FLAG(&hdma_adc, DMA_FLAG_TC1)) {
		__HAL_DMA_CLEAR_FLAG(&hdma_adc, DMA_FLAG_TC1);
		mcpwm_foc_adc_int_handler(NULL, 0);
	}

	// Clear any error flags
	if (__HAL_DMA_GET_FLAG(&hdma_adc, DMA_FLAG_TE1)) {
		__HAL_DMA_CLEAR_FLAG(&hdma_adc, DMA_FLAG_TE1);
	}
}

// ADC1/2 injected end-of-conversion interrupt
void ADC1_2_IRQHandler(void) {
	if (ADC1->SR & ADC_SR_JEOC) {
		ADC1->SR &= ~ADC_SR_JEOC;
		mc_interface_adc_inj_int_handler();
	}
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

// TIM2 CC2 interrupt: FOC sample timing
void TIM2_IRQHandler(void) {
	if (LL_TIM_IsActiveFlag_CC2(TIM2)) {
		mcpwm_foc_tim_sample_int_handler();
		LL_TIM_ClearFlag_CC2(TIM2);
	}
	LL_TIM_ClearFlag_CC2(TIM2);
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

void MemManage_Handler(void) {
	main_stop_motor_and_reset();
}

void BusFault_Handler(void) {
	main_stop_motor_and_reset();
}

void UsageFault_Handler(void) {
	main_stop_motor_and_reset();
}
