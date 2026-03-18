#include "stm32f1xx_hal.h"

COMP_HandleTypeDef hcomp1, hcomp2;
volatile int fault_occurred = 0;
volatile int fault_code = 0;
#define FAULT_OCP 0x01

void mcpwm_hal_fault_init(void) {
    // Comparator 1: Motor 1 OCP (gate shutdown)
    hcomp1.Instance = COMP1;
    hcomp1.Init.InvertingInput = COMP_INVERTINGINPUT_VREFINT;  // Ref: 1.2V
    hcomp1.Init.Output = COMP_OUTPUT_TIM1BKIN;  // Auto-shutdown TIM1
    hcomp1.Init.Mode = COMP_MODE_HIGHSPEED;
    HAL_COMP_Init(&hcomp1);
  
    // Comparator 2: Motor 2 OCP
    hcomp2.Instance = COMP2;
    hcomp2.Init.InvertingInput = COMP_INVERTINGINPUT_VREFINT;
    hcomp2.Init.Output = COMP_OUTPUT_TIM8BKIN;  // Auto-shutdown TIM8
    hcomp2.Init.Mode = COMP_MODE_HIGHSPEED;
    HAL_COMP_Init(&hcomp2);
  
    HAL_COMP_Start(&hcomp1);
    HAL_COMP_Start(&hcomp2);
}

// ISR for external fault input (if GPIO-based)
void EXTI_OCP_IRQHandler(void) {
    extern TIM_HandleTypeDef htim1, htim8;
    // Disable PWM outputs
    __HAL_TIM_MOE_DISABLE(&htim1);  // Master Output Disable
    __HAL_TIM_MOE_DISABLE(&htim8);
  
    // Log fault and trigger recovery sequence
    fault_occurred = 1;
    fault_code = FAULT_OCP;
}
