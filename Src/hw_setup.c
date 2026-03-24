/**
  ******************************************************************************
  * @file    hw_setup.c
  * @brief   Hardware peripheral initialization for Hoverboard ESC
  *          GPIO, TIM1, TIM8, TIM4, TIM2, ADC1/ADC2, USART3, DMA, NVIC, IWDG
  ******************************************************************************
  */

#include "hw_config.h"
#include "stm32f1xx_hal.h"

/* ADC DMA buffer — filled by ADC DMA transfers, indexed by ADC_IND_* macros */
volatile uint16_t ADC_Value[HW_ADC_CHANNELS] = {0};

/* Dummy I2C handle (no I2C hardware on hoverboard, but VESC code references it) */
I2C_HandleTypeDef hi2c1;

/* UART SerialDriver instance for VESC compatibility (wraps huart3) */
#include "ch.h"
SerialDriver HW_UART_DEV_inst;

/* Global peripheral handles */
TIM_HandleTypeDef htim1;    /* Right motor PWM */
TIM_HandleTypeDef htim8;    /* Left motor PWM  */
TIM_HandleTypeDef htim4;    /* HAL tick base   */
TIM_HandleTypeDef htim2;    /* PPM input capture (PB3 / TIM2_CH2) */
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
UART_HandleTypeDef huart3;  /* VESC communication */
IWDG_HandleTypeDef hiwdg;   /* Independent watchdog */
CAN_HandleTypeDef hcan1;    /* CAN bus (stub for compatibility) */
DMA_HandleTypeDef hdma_usart3_rx;
DMA_HandleTypeDef hdma_usart3_tx;

/*============================================================================
 * GPIO Initialization
 *============================================================================*/
void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();

    /* --- Hall Sensors (Input) --- */
    GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    GPIO_InitStruct.Pin = LEFT_HALL_U_PIN;
    HAL_GPIO_Init(LEFT_HALL_U_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = LEFT_HALL_V_PIN;
    HAL_GPIO_Init(LEFT_HALL_V_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = LEFT_HALL_W_PIN;
    HAL_GPIO_Init(LEFT_HALL_W_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = RIGHT_HALL_U_PIN;
    HAL_GPIO_Init(RIGHT_HALL_U_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = RIGHT_HALL_V_PIN;
    HAL_GPIO_Init(RIGHT_HALL_V_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = RIGHT_HALL_W_PIN;
    HAL_GPIO_Init(RIGHT_HALL_W_PORT, &GPIO_InitStruct);

    /* --- Control outputs --- */
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;

    GPIO_InitStruct.Pin = LED_PIN;
    HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = BUZZER_PIN;
    HAL_GPIO_Init(BUZZER_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = OFF_PIN;
    HAL_GPIO_Init(OFF_PORT, &GPIO_InitStruct);

    /* --- Button (Input) --- */
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Pin  = BUTTON_PIN;
    HAL_GPIO_Init(BUTTON_PORT, &GPIO_InitStruct);

    /* --- Charger detect (Input pullup) --- */
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Pin  = CHARGER_PIN;
    HAL_GPIO_Init(CHARGER_PORT, &GPIO_InitStruct);

    /* --- Analog pins (ADC) --- */
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;

    GPIO_InitStruct.Pin = LEFT_DC_CUR_PIN;
    HAL_GPIO_Init(LEFT_DC_CUR_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = LEFT_U_CUR_PIN;
    HAL_GPIO_Init(LEFT_U_CUR_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = LEFT_V_CUR_PIN;
    HAL_GPIO_Init(LEFT_V_CUR_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = RIGHT_DC_CUR_PIN;
    HAL_GPIO_Init(RIGHT_DC_CUR_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = RIGHT_U_CUR_PIN;
    HAL_GPIO_Init(RIGHT_U_CUR_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = RIGHT_V_CUR_PIN;
    HAL_GPIO_Init(RIGHT_V_CUR_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = DCLINK_PIN;
    HAL_GPIO_Init(DCLINK_PORT, &GPIO_InitStruct);

    /* --- PWM pins (AF Push-Pull) --- */
    GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    /* Right motor (TIM1) */
    GPIO_InitStruct.Pin = RIGHT_TIM_UH_PIN;
    HAL_GPIO_Init(RIGHT_TIM_UH_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = RIGHT_TIM_VH_PIN;
    HAL_GPIO_Init(RIGHT_TIM_VH_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = RIGHT_TIM_WH_PIN;
    HAL_GPIO_Init(RIGHT_TIM_WH_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = RIGHT_TIM_UL_PIN;
    HAL_GPIO_Init(RIGHT_TIM_UL_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = RIGHT_TIM_VL_PIN;
    HAL_GPIO_Init(RIGHT_TIM_VL_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = RIGHT_TIM_WL_PIN;
    HAL_GPIO_Init(RIGHT_TIM_WL_PORT, &GPIO_InitStruct);

    /* Left motor (TIM8) */
    GPIO_InitStruct.Pin = LEFT_TIM_UH_PIN;
    HAL_GPIO_Init(LEFT_TIM_UH_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = LEFT_TIM_VH_PIN;
    HAL_GPIO_Init(LEFT_TIM_VH_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = LEFT_TIM_WH_PIN;
    HAL_GPIO_Init(LEFT_TIM_WH_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = LEFT_TIM_UL_PIN;
    HAL_GPIO_Init(LEFT_TIM_UL_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = LEFT_TIM_VL_PIN;
    HAL_GPIO_Init(LEFT_TIM_VL_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = LEFT_TIM_WL_PIN;
    HAL_GPIO_Init(LEFT_TIM_WL_PORT, &GPIO_InitStruct);
}

/*============================================================================
 * DMA Initialization
 *============================================================================*/
void MX_DMA_Init(void)
{
    __HAL_RCC_DMA1_CLK_ENABLE();

    /* DMA1_Channel1 — ADC1 DMA */
    HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

    /* DMA1_Channel2 — USART3_TX */
    HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);

    /* DMA1_Channel3 — USART3_RX */
    HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);
}

/*============================================================================
 * TIM1 — Right Motor PWM (Master)
 *============================================================================*/
void MX_TIM1_Init(void)
{
    __HAL_RCC_TIM1_CLK_ENABLE();

    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

    uint16_t pwm_period = (uint16_t)(SYSCLK_FREQ / 2 / PWM_FREQUENCY);

    htim1.Instance               = TIM1;
    htim1.Init.Prescaler         = 0;
    htim1.Init.CounterMode       = TIM_COUNTERMODE_CENTERALIGNED1;
    htim1.Init.Period            = pwm_period;
    htim1.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_PWM_Init(&htim1);

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_ENABLE;
    sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig);

    sConfigOC.OCMode       = TIM_OCMODE_PWM1;
    sConfigOC.Pulse        = 0;
    sConfigOC.OCPolarity   = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity  = TIM_OCNPOLARITY_LOW;
    sConfigOC.OCFastMode   = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState  = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_SET;
    HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1);
    HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2);
    HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3);

    /* CH4 for ADC trigger */
    sConfigOC.OCMode = TIM_OCMODE_PWM2;
    sConfigOC.Pulse  = pwm_period - 1;
    HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_4);

    sBreakDeadTimeConfig.OffStateRunMode  = TIM_OSSR_ENABLE;
    sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_ENABLE;
    sBreakDeadTimeConfig.LockLevel        = TIM_LOCKLEVEL_OFF;
    sBreakDeadTimeConfig.DeadTime         = 32;  /* ~500ns at 64MHz */
    sBreakDeadTimeConfig.BreakState       = TIM_BREAK_DISABLE;
    sBreakDeadTimeConfig.BreakPolarity    = TIM_BREAKPOLARITY_LOW;
    sBreakDeadTimeConfig.AutomaticOutput  = TIM_AUTOMATICOUTPUT_DISABLE;
    HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig);
}

/*============================================================================
 * TIM8 — Left Motor PWM (Gated Slave of TIM1)
 *============================================================================*/
void MX_TIM8_Init(void)
{
    __HAL_RCC_TIM8_CLK_ENABLE();

    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_SlaveConfigTypeDef  sSlaveConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

    uint16_t pwm_period = (uint16_t)(SYSCLK_FREQ / 2 / PWM_FREQUENCY);

    htim8.Instance               = TIM8;
    htim8.Init.Prescaler         = 0;
    htim8.Init.CounterMode       = TIM_COUNTERMODE_CENTERALIGNED1;
    htim8.Init.Period            = pwm_period;
    htim8.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim8.Init.RepetitionCounter = 0;
    htim8.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_PWM_Init(&htim8);

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_ENABLE;
    HAL_TIMEx_MasterConfigSynchronization(&htim8, &sMasterConfig);

    sSlaveConfig.InputTrigger = TIM_TS_ITR0;  /* TIM8 slaved to TIM1 */
    sSlaveConfig.SlaveMode    = TIM_SLAVEMODE_GATED;
    HAL_TIM_SlaveConfigSynchronization(&htim8, &sSlaveConfig);

    /* Offset TIM8 counter for proper ADC alignment */
    TIM8->CNT = 20;

    sConfigOC.OCMode       = TIM_OCMODE_PWM1;
    sConfigOC.Pulse        = 0;
    sConfigOC.OCPolarity   = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity  = TIM_OCNPOLARITY_LOW;
    sConfigOC.OCFastMode   = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState  = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_SET;
    HAL_TIM_PWM_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_1);
    HAL_TIM_PWM_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_2);
    HAL_TIM_PWM_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_3);

    sBreakDeadTimeConfig.OffStateRunMode  = TIM_OSSR_ENABLE;
    sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_ENABLE;
    sBreakDeadTimeConfig.LockLevel        = TIM_LOCKLEVEL_OFF;
    sBreakDeadTimeConfig.DeadTime         = 32;
    sBreakDeadTimeConfig.BreakState       = TIM_BREAK_DISABLE;
    sBreakDeadTimeConfig.BreakPolarity    = TIM_BREAKPOLARITY_LOW;
    sBreakDeadTimeConfig.AutomaticOutput  = TIM_AUTOMATICOUTPUT_DISABLE;
    HAL_TIMEx_ConfigBreakDeadTime(&htim8, &sBreakDeadTimeConfig);
}

/*============================================================================
 * TIM4 — HAL Tick Base (1 kHz) — avoids SysTick conflict with FreeRTOS
 *============================================================================*/
void MX_TIM4_Init(void)
{
    __HAL_RCC_TIM4_CLK_ENABLE();

    htim4.Instance = TIM4;
    htim4.Init.Prescaler         = (uint32_t)(SYSCLK_FREQ / 1000000) - 1;  /* 1 MHz tick */
    htim4.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim4.Init.Period            = 1000 - 1;  /* 1 kHz overflow */
    htim4.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_Base_Init(&htim4);

    HAL_NVIC_SetPriority(TIM4_IRQn, 15, 0);
    HAL_NVIC_EnableIRQ(TIM4_IRQn);

    HAL_TIM_Base_Start_IT(&htim4);
}

/*============================================================================
 * ADC1 — Right motor currents + battery + temperature
 *============================================================================*/
void MX_ADC1_Init(void)
{
    ADC_MultiModeTypeDef multimode = {0};
    ADC_ChannelConfTypeDef sConfig = {0};

    __HAL_RCC_ADC1_CLK_ENABLE();

    hadc1.Instance                   = ADC1;
    hadc1.Init.ScanConvMode          = ADC_SCAN_ENABLE;
    hadc1.Init.ContinuousConvMode    = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv      = ADC_EXTERNALTRIGCONV_T8_TRGO;
    hadc1.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion       = 5;
    HAL_ADC_Init(&hadc1);

    __HAL_AFIO_REMAP_ADC1_ETRGREG_ENABLE();

    multimode.Mode = ADC_DUALMODE_REGSIMULT;
    HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode);

    /* Rank 1: Right DC current (PC1 = CH11) */
    sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
    sConfig.Channel = ADC_CHANNEL_11;
    sConfig.Rank    = 1;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    /* Rank 2: Left phase A current (PA0 = CH0) */
    sConfig.SamplingTime = ADC_SAMPLETIME_7CYCLES_5;
    sConfig.Channel = ADC_CHANNEL_0;
    sConfig.Rank    = 2;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    /* Rank 3: Right phase B current (PC4 = CH14) */
    sConfig.Channel = ADC_CHANNEL_14;
    sConfig.Rank    = 3;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    /* Rank 4: Battery voltage (PC2 = CH12) */
    sConfig.Channel = ADC_CHANNEL_12;
    sConfig.Rank    = 4;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    /* Rank 5: Internal temperature sensor */
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
    sConfig.Rank    = 5;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    hadc1.Instance->CR2 |= ADC_CR2_DMA | ADC_CR2_TSVREFE;
    __HAL_ADC_ENABLE(&hadc1);
}

/*============================================================================
 * ADC2 — Left motor currents
 *============================================================================*/
void MX_ADC2_Init(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    __HAL_RCC_ADC2_CLK_ENABLE();

    hadc2.Instance                   = ADC2;
    hadc2.Init.ScanConvMode          = ADC_SCAN_ENABLE;
    hadc2.Init.ContinuousConvMode    = DISABLE;
    hadc2.Init.DiscontinuousConvMode = DISABLE;
    hadc2.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
    hadc2.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    hadc2.Init.NbrOfConversion       = 5;
    HAL_ADC_Init(&hadc2);

    /* Rank 1: Left DC current (PC0 = CH10) */
    sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
    sConfig.Channel = ADC_CHANNEL_10;
    sConfig.Rank    = 1;
    HAL_ADC_ConfigChannel(&hadc2, &sConfig);

    /* Rank 2: Left phase B current (PC3 = CH13) */
    sConfig.SamplingTime = ADC_SAMPLETIME_7CYCLES_5;
    sConfig.Channel = ADC_CHANNEL_13;
    sConfig.Rank    = 2;
    HAL_ADC_ConfigChannel(&hadc2, &sConfig);

    /* Rank 3: Right phase C current (PC5 = CH15) */
    sConfig.Channel = ADC_CHANNEL_15;
    sConfig.Rank    = 3;
    HAL_ADC_ConfigChannel(&hadc2, &sConfig);

    /* Rank 4: analog input ch2 */
    sConfig.Channel = ADC_CHANNEL_2;
    sConfig.Rank    = 4;
    HAL_ADC_ConfigChannel(&hadc2, &sConfig);

    /* Rank 5: analog input ch3 */
    sConfig.Channel = ADC_CHANNEL_3;
    sConfig.Rank    = 5;
    HAL_ADC_ConfigChannel(&hadc2, &sConfig);

    hadc2.Instance->CR2 |= ADC_CR2_DMA;
    __HAL_ADC_ENABLE(&hadc2);
}

/*============================================================================
 * USART3 — VESC Communication (PB10=TX, PB11=RX)
 *============================================================================*/
void MX_USART3_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_USART3_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* TX = PB10 */
    GPIO_InitStruct.Pin   = GPIO_PIN_10;
    GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* RX = PB11 */
    GPIO_InitStruct.Pin  = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    huart3.Instance          = USART3;
    huart3.Init.BaudRate     = VESC_USART_BAUD;
    huart3.Init.WordLength   = UART_WORDLENGTH_8B;
    huart3.Init.StopBits     = UART_STOPBITS_1;
    huart3.Init.Parity       = UART_PARITY_NONE;
    huart3.Init.Mode         = UART_MODE_TX_RX;
    huart3.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    huart3.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart3);

    /* DMA for USART3 RX */
    hdma_usart3_rx.Instance                 = DMA1_Channel3;
    hdma_usart3_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hdma_usart3_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_usart3_rx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_usart3_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart3_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_usart3_rx.Init.Mode                = DMA_CIRCULAR;
    hdma_usart3_rx.Init.Priority            = DMA_PRIORITY_LOW;
    HAL_DMA_Init(&hdma_usart3_rx);
    __HAL_LINKDMA(&huart3, hdmarx, hdma_usart3_rx);

    /* DMA for USART3 TX */
    hdma_usart3_tx.Instance                 = DMA1_Channel2;
    hdma_usart3_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    hdma_usart3_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_usart3_tx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_usart3_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart3_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_usart3_tx.Init.Mode                = DMA_NORMAL;
    hdma_usart3_tx.Init.Priority            = DMA_PRIORITY_LOW;
    HAL_DMA_Init(&hdma_usart3_tx);
    __HAL_LINKDMA(&huart3, hdmatx, hdma_usart3_tx);

    HAL_NVIC_SetPriority(USART3_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);

    __HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);
}

/*============================================================================
 * NVIC Initialization
 *============================================================================*/
void MX_NVIC_Init(void)
{
    /* TIM1 Update — FOC high frequency */
    HAL_NVIC_SetPriority(TIM1_UP_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM1_UP_IRQn);

    /* TIM1 Break — Overcurrent */
    HAL_NVIC_SetPriority(TIM1_BRK_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM1_BRK_IRQn);

    /* ADC1/2 */
    HAL_NVIC_SetPriority(ADC1_2_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(ADC1_2_IRQn);
}

/*============================================================================
 * TIM2 — PPM Input Capture (PB3 = TIM2_CH2, partial remap)
 * Prescaler set to 64-1 = 63 → 1 µs tick at 64 MHz SYSCLK.
 * Input capture on CH2 (PB3 via partial remap).
 *============================================================================*/
void MX_TIM2_Init(void)
{
    __HAL_RCC_TIM2_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();

    /* Partial remap: TIM2_CH2 → PB3 */
    __HAL_AFIO_REMAP_TIM2_PARTIAL_1();

    /* PB3 as input floating (timer captures it) */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitStruct.Pin   = PPM_INPUT_PIN;   /* GPIO_PIN_3 */
    GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull  = GPIO_PULLDOWN;
    HAL_GPIO_Init(PPM_INPUT_PORT, &GPIO_InitStruct);  /* GPIOB */

    /* Timer base: 1 µs resolution, 32-bit free-running */
    htim2.Instance               = TIM2;
    htim2.Init.Prescaler         = (uint32_t)(SYSCLK_FREQ / 1000000UL) - 1U;
    htim2.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim2.Init.Period             = 0xFFFFFFFFUL;
    htim2.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_IC_Init(&htim2);

    /* CH2 input capture configured in servo_dec_init() */

    /* Enable TIM2 interrupt */
    HAL_NVIC_SetPriority(TIM2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
}

/*============================================================================
 * IWDG — Independent Watchdog
 * LSI ~40 kHz, prescaler /32 → ~1.25 kHz tick.
 * Reload = 140 → ~112 ms timeout.
 * Application must call HAL_IWDG_Refresh(&hiwdg) at least every 100 ms.
 *============================================================================*/
void MX_IWDG_Init(void)
{
    hiwdg.Instance       = IWDG;
    hiwdg.Init.Prescaler = IWDG_PRESCALER_32;
    hiwdg.Init.Reload    = IWDG_RELOAD_VALUE;  /* ~112 ms */
    HAL_IWDG_Init(&hiwdg);
}

/*============================================================================
 * Start All PWM Timers
 *============================================================================*/
void startTimers(void)
{
    /* Disable MOE initially */
    TIM1->BDTR &= ~TIM_BDTR_MOE;
    TIM8->BDTR &= ~TIM_BDTR_MOE;

    /* Start Left (TIM8) PWM channels */
    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_3);
    HAL_TIMEx_PWMN_Start(&htim8, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Start(&htim8, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Start(&htim8, TIM_CHANNEL_3);

    /* Start Right (TIM1) PWM channels */
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);

    /* RCR for TIM8 to sync ADC triggers */
    htim8.Instance->RCR = 1;

    /* Enable TIM1 (master) — TIM8 follows via gated slave */
    __HAL_TIM_ENABLE(&htim1);
}
