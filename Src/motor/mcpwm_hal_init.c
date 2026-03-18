#include "stm32f1xx_hal.h"
#include "stm32f1xx_ll_tim.h"

// Timer handles (global)
extern TIM_HandleTypeDef htim1, htim8;
extern ADC_HandleTypeDef hadc1;

void mcpwm_init_hardware(void) {
    // ===== TIM1 MOTOR 1 PWM =====
    __HAL_RCC_TIM1_CLK_ENABLE();
    
    htim1.Instance = TIM1;
    htim1.Init.Prescaler = 0;
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = 4500 - 1;  // 16 kHz
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_PWM_Init(&htim1);
    
    // PWM channels 1, 2, 3
    TIM_OC_InitTypeDef sConfigOC = {0};
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 2250;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_ENABLE;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    
    HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1);
    HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2);
    HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3);
    
    // Break & Dead Time
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};
    sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_ENABLE;
    sBreakDeadTimeConfig.OffStateIdleMode = TIM_OSSI_ENABLE;
    sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
    sBreakDeadTimeConfig.DeadTime = 72;  // 1 us
    sBreakDeadTimeConfig.BreakState = TIM_BREAK_ENABLE;
    sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_LOW;
    sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_ENABLE;
    HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig);
    
    // Start PWM
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);
    
    // ===== TIM8 MOTOR 2 PWM (SLAVE) =====
    __HAL_RCC_TIM8_CLK_ENABLE();
    
    htim8.Instance = TIM8;
    htim8.Init.Prescaler = 0;
    htim8.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim8.Init.Period = 4500 - 1;
    htim8.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim8.Init.RepetitionCounter = 0;
    HAL_TIM_PWM_Init(&htim8);
    
    // Slave mode trigger from TIM1
    LL_TIM_SetSlaveMode(TIM8, LL_TIM_SLAVEMODE_TRIGGER);
    LL_TIM_SetTriggerInput(TIM8, LL_TIM_TS_ITR0);
    
    // Phase offset 180 degrees
    TIM8->CNT = 2250;
    
    // Same PWM config
    HAL_TIM_PWM_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_1);
    HAL_TIM_PWM_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_2);
    HAL_TIM_PWM_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_3);
    HAL_TIMEx_ConfigBreakDeadTime(&htim8, &sBreakDeadTimeConfig);
    
    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_3);
    HAL_TIMEx_PWMN_Start(&htim8, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Start(&htim8, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Start(&htim8, TIM_CHANNEL_3);
    
    // ===== ADC1 SETUP =====
    // NOTE: Full ADC initialization (with DMA, callbacks) happens in mcpwm_foc_init()
    // This section ensures ADC clock is enabled early
    __HAL_RCC_ADC1_CLK_ENABLE();
    
    // ADC will be initialized by mcpwm_foc_init() with:
    // - DMA1_Channel1 for continuous ADC conversion
    // - TIM2_CC2 as external trigger for synchronized sampling
    // - Dual-motor phase-locked ADC triggers from TIM1/TIM8
    
    // Injected channel configuration (alternative high-performance path):
    // - ADC_InjectionConfTypeDef for 3 channels (IU, IV, IW)
    // - External trigger: ADC_EXTERNALTRIGINJECCONV_T1_TRGO
    // - This provides <2 µs ISR latency for motor control feedback
    
    // ADC calibration happens post-clock-configuration in mcpwm_foc_init
}

/**
 * Dual-Motor ADC Synchronization Setup
 * 
 * For dual motors, ensure:
 * 1. ADC1 samples Motor 1 currents at TIM1_CC4 trigger (6 kHz ISR)
 * 2. TIM1 master output (TRGO) enables injected conversions
 * 3. TIM8 slave mode (trigger from TIM1) maintains 180° phase offset
 * 4. Both motors sampled at same electrical phase for FOC control
 * 
 * Phase Lock Verification:
 * - TIM1->CNT should lead TIM8->CNT by ~2250 counts (180°)
 * - Call hw_verify_dual_motor_sync() in diagnostics
 * 
 * ISR Latency Target: <2 µs from ADC trigger to PWM duty update
 * - Achieved by direct DMA->motor ISR callback path
 * - No FreeRTOS mutex/semaphore overhead in critical section
 */
void mcpwm_hal_adc_dual_sync_verify(void) {
    // Verify TIM1/TIM8 phase lock at runtime
    extern void hw_verify_dual_motor_sync(void);
    hw_verify_dual_motor_sync();
}
