#ifndef ISR_VECTOR_TABLE_H_
#define ISR_VECTOR_TABLE_H_

/*
 * STM32F1 ISR Vector Table
 *
 * On STM32F1 with HAL, interrupt handlers use standard CMSIS names directly.
 * This header provides compatibility defines for code that references
 * F4-style vector names.
 *
 * Note: STM32F1 has ADC1_2_IRQHandler (not ADC1_2_3_IRQHandler since F1
 * doesn't have ADC3 on most variants, but high-density F103 does).
 *
 * ISR handlers are defined directly in irq_handlers.c using standard names.
 */

/* STM32F1 does not have DMA streams - it has DMA channels */
/* DMA1_Channel1 is used for ADC1 */

/* Timer IRQs - same names on F1 as F4 */
/* TIM1_BRK_IRQHandler - TIM1 Break */
/* TIM1_UP_IRQHandler - TIM1 Update */
/* TIM1_TRG_COM_IRQHandler - TIM1 Trigger and Commutation */
/* TIM1_CC_IRQHandler - TIM1 Capture Compare */
/* TIM2_IRQHandler - TIM2 */
/* TIM8_BRK_IRQHandler - TIM8 Break */
/* TIM8_UP_IRQHandler - TIM8 Update */
/* TIM8_TRG_COM_IRQHandler - TIM8 Trigger and Commutation */
/* TIM8_CC_IRQHandler - TIM8 Capture Compare */

#endif /* ISR_VECTOR_TABLE_H_ */
