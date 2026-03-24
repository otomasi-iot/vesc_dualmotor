/*
 * STM32F4 to STM32F1 Compatibility Layer
 * Provides STM32F4 StdPeriph API compatibility for STM32F1 HAL
 * This allows VESC motor control code to compile on F103
 */

#ifndef STM32F4_COMPAT_H
#define STM32F4_COMPAT_H

#include "stm32f1xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 * RCC - Clock Control Compatibility
 *============================================================================*/

// AHB1 Peripherals (F4) → AHB Peripherals (F1)
#define RCC_AHB1Periph_DMA1         RCC_AHBENR_DMA1EN
#define RCC_AHB1Periph_DMA2         RCC_AHBENR_DMA2EN
#define RCC_AHB1Periph_GPIOA        RCC_APB2ENR_IOPAEN
#define RCC_AHB1Periph_GPIOB        RCC_APB2ENR_IOPBEN
#define RCC_AHB1Periph_GPIOC        RCC_APB2ENR_IOPCEN
#define RCC_AHB1Periph_GPIOD        RCC_APB2ENR_IOPDEN
#define RCC_AHB1Periph_GPIOE        RCC_APB2ENR_IOPEEN

// APB2 Peripherals
#define RCC_APB2Periph_ADC1         RCC_APB2ENR_ADC1EN
#define RCC_APB2Periph_ADC2         RCC_APB2ENR_ADC2EN
#define RCC_APB2Periph_ADC3         0  // F103 doesn't have ADC3
#define RCC_APB2Periph_TIM1         RCC_APB2ENR_TIM1EN
#define RCC_APB2Periph_TIM8         RCC_APB2ENR_TIM8EN

// APB1 Peripherals
#define RCC_APB1Periph_TIM2         RCC_APB1ENR_TIM2EN
#define RCC_APB1Periph_TIM3         RCC_APB1ENR_TIM3EN
#define RCC_APB1Periph_TIM4         RCC_APB1ENR_TIM4EN

// RCC Functions
static inline void RCC_AHB1PeriphClockCmd(uint32_t RCC_AHB1Periph, FunctionalState NewState) {
    if (NewState != DISABLE) {
        if (RCC_AHB1Periph & (RCC_AHBENR_DMA1EN | RCC_AHBENR_DMA2EN)) {
            RCC->AHBENR |= (RCC_AHB1Periph & (RCC_AHBENR_DMA1EN | RCC_AHBENR_DMA2EN));
        }
        if (RCC_AHB1Periph & (RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN | 
                              RCC_APB2ENR_IOPDEN | RCC_APB2ENR_IOPEEN)) {
            RCC->APB2ENR |= (RCC_AHB1Periph & (RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | 
                                                RCC_APB2ENR_IOPCEN | RCC_APB2ENR_IOPDEN | RCC_APB2ENR_IOPEEN));
        }
    } else {
        if (RCC_AHB1Periph & (RCC_AHBENR_DMA1EN | RCC_AHBENR_DMA2EN)) {
            RCC->AHBENR &= ~(RCC_AHB1Periph & (RCC_AHBENR_DMA1EN | RCC_AHBENR_DMA2EN));
        }
        if (RCC_AHB1Periph & (RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN | 
                              RCC_APB2ENR_IOPDEN | RCC_APB2ENR_IOPEEN)) {
            RCC->APB2ENR &= ~(RCC_AHB1Periph & (RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | 
                                                 RCC_APB2ENR_IOPCEN | RCC_APB2ENR_IOPDEN | RCC_APB2ENR_IOPEEN));
        }
    }
}

static inline void RCC_APB2PeriphClockCmd(uint32_t RCC_APB2Periph, FunctionalState NewState) {
    if (NewState != DISABLE) {
        RCC->APB2ENR |= RCC_APB2Periph;
    } else {
        RCC->APB2ENR &= ~RCC_APB2Periph;
    }
}

static inline void RCC_APB1PeriphClockCmd(uint32_t RCC_APB1Periph, FunctionalState NewState) {
    if (NewState != DISABLE) {
        RCC->APB1ENR |= RCC_APB1Periph;
    } else {
        RCC->APB1ENR &= ~RCC_APB1Periph;
    }
}

/*============================================================================
 * DMA - Stream to Channel Mapping
 *============================================================================*/

// F4 uses DMA2_Stream4, F1 uses DMA2_Channel5 for ADC
#define DMA2_Stream4                DMA2_Channel5
#define DMA2_Stream0                DMA2_Channel1

// DMA Channel definitions (F4 has channels within streams, F1 doesn't)
#define DMA_Channel_0               0

// DMA Direction
#define DMA_DIR_PeripheralToMemory  DMA_PERIPH_TO_MEMORY
#define DMA_DIR_MemoryToPeripheral  DMA_MEMORY_TO_PERIPH

// DMA Increment
#define DMA_PeripheralInc_Disable   0
#define DMA_PeripheralInc_Enable    DMA_PINC_ENABLE
#define DMA_MemoryInc_Enable        DMA_MINC_ENABLE
#define DMA_MemoryInc_Disable       0

// DMA Data Size
#define DMA_PeripheralDataSize_HalfWord  DMA_PDATAALIGN_HALFWORD
#define DMA_MemoryDataSize_HalfWord      DMA_MDATAALIGN_HALFWORD
#define DMA_PeripheralDataSize_Word      DMA_PDATAALIGN_WORD
#define DMA_MemoryDataSize_Word          DMA_MDATAALIGN_WORD

// DMA Mode
#define DMA_Mode_Circular           DMA_CIRCULAR
#define DMA_Mode_Normal             DMA_NORMAL

// DMA Priority
#define DMA_Priority_High           DMA_PRIORITY_HIGH
#define DMA_Priority_VeryHigh       DMA_PRIORITY_VERY_HIGH

// DMA FIFO (F1 doesn't have FIFO, these are ignored)
#define DMA_FIFOMode_Disable        0
#define DMA_FIFOMode_Enable         0
#define DMA_FIFOThreshold_1QuarterFull  0
#define DMA_MemoryBurst_Single      0
#define DMA_PeripheralBurst_Single  0

// DMA Init Structure (F4 to F1 mapping)
typedef struct {
    uint32_t DMA_Channel;              // Ignored in F1
    uint32_t DMA_PeripheralBaseAddr;   // Maps to PeriphAddress
    uint32_t DMA_Memory0BaseAddr;      // Maps to MemAddress
    uint32_t DMA_DIR;                  // Maps to Direction
    uint32_t DMA_BufferSize;           // Maps to DataLength
    uint32_t DMA_PeripheralInc;        // Maps to PeriphInc
    uint32_t DMA_MemoryInc;            // Maps to MemInc
    uint32_t DMA_PeripheralDataSize;   // Maps to PeriphDataAlignment
    uint32_t DMA_MemoryDataSize;       // Maps to MemDataAlignment
    uint32_t DMA_Mode;                 // Maps to Mode
    uint32_t DMA_Priority;             // Maps to Priority
    uint32_t DMA_FIFOMode;             // Ignored
    uint32_t DMA_FIFOThreshold;        // Ignored
    uint32_t DMA_MemoryBurst;          // Ignored
    uint32_t DMA_PeripheralBurst;      // Ignored
} DMA_InitTypeDef_F4;

void DMA_Init(DMA_Channel_TypeDef* DMAy_Channelx, DMA_InitTypeDef_F4* DMA_InitStruct);
void DMA_Cmd(DMA_Channel_TypeDef* DMAy_Channelx, FunctionalState NewState);
void DMA_ITConfig(DMA_Channel_TypeDef* DMAy_Channelx, uint32_t DMA_IT, FunctionalState NewState);
void DMA_DeInit(DMA_Channel_TypeDef* DMAy_Channelx);

// Redefine DMA_InitTypeDef to F4 version for compatibility
#define DMA_InitTypeDef DMA_InitTypeDef_F4

/*============================================================================
 * ADC - Triple Mode to Dual Mode Compatibility
 *============================================================================*/

// ADC Common structure (F4 only, F1 doesn't have this)
typedef struct {
    uint32_t ADC_Mode;
    uint32_t ADC_Prescaler;
    uint32_t ADC_DMAAccessMode;
    uint32_t ADC_TwoSamplingDelay;
} ADC_CommonInitTypeDef_F4;

#define ADC_CommonInitTypeDef ADC_CommonInitTypeDef_F4

// ADC Modes (F1 only has dual mode)
#define ADC_TripleMode_RegSimult    ADC_DUALMODE_REGSIMULT
#define ADC_Mode_Independent        0

// ADC Prescaler
#define ADC_Prescaler_Div2          RCC_ADCPCLK2_DIV2
#define ADC_Prescaler_Div4          RCC_ADCPCLK2_DIV4
#define ADC_Prescaler_Div6          RCC_ADCPCLK2_DIV6
#define ADC_Prescaler_Div8          RCC_ADCPCLK2_DIV8

// ADC DMA Access Mode (F1 doesn't have this concept)
#define ADC_DMAAccessMode_1         0
#define ADC_DMAAccessMode_Disabled  0

// ADC Sampling Delay (F1 doesn't have this, use dummy value)
#define ADC_TwoSamplingDelay_5Cycles   0

// ADC Resolution (F1 is fixed 12-bit)
#define ADC_Resolution_12b          0

// ADC Scan Mode
#define ADC_ScanConvMode            ScanConvMode

// ADC Continuous Mode
#define ADC_ContinuousConvMode      ContinuousConvMode

// ADC External Trigger Edge
#define ADC_ExternalTrigConvEdge_None      0
#define ADC_ExternalTrigConvEdge_Rising    1
#define ADC_ExternalTrigConvEdge_Falling   2
#define ADC_ExternalTrigConvEdge_RisingFalling  3

// ADC External Trigger (F4 to F1 mapping)
#define ADC_ExternalTrigConv_T8_CC1        ADC_EXTERNALTRIGCONV_T8_TRGO
#define ADC_ExternalTrigConv_T2_CC2        ADC_EXTERNALTRIGCONV_T2_CC2
#define ADC_ExternalTrigConv               ExternalTrigConv

// ADC Injected External Trigger
#define ADC_ExternalTrigInjecConv_T1_CC4   ADC_EXTERNALTRIGINJECCONV_T1_CC4
#define ADC_ExternalTrigInjecConv_T8_CC2   ADC_EXTERNALTRIGINJECCONV_T8_CC4
#define ADC_ExternalTrigInjecConv_T8_CC3   ADC_EXTERNALTRIGINJECCONV_T8_CC4

#define ADC_ExternalTrigInjecConvEdge_None     0
#define ADC_ExternalTrigInjecConvEdge_Rising   1
#define ADC_ExternalTrigInjecConvEdge_Falling  2

// ADC Data Align
#define ADC_DataAlign_Right         ADC_DATAALIGN_RIGHT
#define ADC_DataAlign               DataAlign

// ADC Number of Conversions
#define ADC_NbrOfConversion         NbrOfConversion

// ADC IT - already defined by HAL headers (ADC_IT_JEOC, ADC_IT_AWD)

// ADC IRQ (F103 has ADC1_2_IRQn for ADC1 and ADC2)
#define ADC_IRQn                    ADC1_2_IRQn

// ADC Init structures for F4 StdPeriph compatibility
typedef struct {
    uint32_t ADC_Resolution;
    uint32_t ADC_ScanConvMode;
    uint32_t ADC_ContinuousConvMode;
    uint32_t ADC_ExternalTrigConvEdge;
    uint32_t ADC_ExternalTrigConv;
    uint32_t ADC_DataAlign;
    uint8_t ADC_NbrOfConversion;
} ADC_InitTypeDef_F4;

#define ADC_InitTypeDef ADC_InitTypeDef_F4

// ADC Init function
void ADC_Init(ADC_TypeDef* ADCx, ADC_InitTypeDef_F4* ADC_InitStruct);
void ADC_CommonInit(ADC_CommonInitTypeDef_F4* ADC_CommonInitStruct);
void ADC_Cmd(ADC_TypeDef* ADCx, FunctionalState NewState);
void ADC_DeInit(void);
void ADC_TempSensorVrefintCmd(FunctionalState NewState);
void ADC_MultiModeDMARequestAfterLastTransferCmd(FunctionalState NewState);
void ADC_ExternalTrigInjectedConvConfig(ADC_TypeDef* ADCx, uint32_t ADC_ExternalTrigInjecConv);
void ADC_ExternalTrigInjectedConvEdgeConfig(ADC_TypeDef* ADCx, uint32_t ADC_ExternalTrigInjecConvEdge);
void ADC_InjectedSequencerLengthConfig(ADC_TypeDef* ADCx, uint8_t Length);
void ADC_ITConfig(ADC_TypeDef* ADCx, uint16_t ADC_IT, FunctionalState NewState);

// ADC Common register (F4 only, create dummy for F1)
typedef struct {
    volatile uint32_t CDR;  // Common data register
} ADC_Common_TypeDef_F4;

extern ADC_Common_TypeDef_F4* ADC;

/*============================================================================
 * TIM - Timer Compatibility (mostly compatible)
 *============================================================================*/

// Timer Counter Mode
#define TIM_CounterMode_Up              TIM_COUNTERMODE_UP
#define TIM_CounterMode_CenterAligned1  TIM_COUNTERMODE_CENTERALIGNED1

// Timer OC Mode
#define TIM_OCMode_PWM1                 TIM_OCMODE_PWM1
#define TIM_OCMode_PWM2                 TIM_OCMODE_PWM2
#define TIM_OCMode_Inactive             TIM_OCMODE_INACTIVE

// Timer Output State
#define TIM_OutputState_Enable          TIM_OUTPUTSTATE_ENABLE
#define TIM_OutputState_Disable         TIM_OUTPUTSTATE_DISABLE
#define TIM_OutputNState_Enable         TIM_OUTPUTNSTATE_ENABLE
#define TIM_OutputNState_Disable        TIM_OUTPUTNSTATE_DISABLE

// Timer Polarity
#define TIM_OCPolarity_High             TIM_OCPOLARITY_HIGH
#define TIM_OCPolarity_Low              TIM_OCPOLARITY_LOW
#define TIM_OCNPolarity_High            TIM_OCNPOLARITY_HIGH
#define TIM_OCNPolarity_Low             TIM_OCNPOLARITY_LOW

// Timer Idle State
#define TIM_OCIdleState_Set             TIM_OCIDLESTATE_SET
#define TIM_OCIdleState_Reset           TIM_OCIDLESTATE_RESET
#define TIM_OCNIdleState_Set            TIM_OCNIDLESTATE_SET
#define TIM_OCNIdleState_Reset          TIM_OCNIDLESTATE_RESET

// Timer BDTR
#define TIM_OSSRState_Enable            TIM_OSSR_ENABLE
#define TIM_OSSIState_Enable            TIM_OSSI_ENABLE
#define TIM_LOCKLevel_OFF               TIM_LOCKLEVEL_OFF
#define TIM_Break_Enable                TIM_BREAK_ENABLE
#define TIM_Break_Disable               TIM_BREAK_DISABLE
#define TIM_BreakPolarity_High          TIM_BREAKPOLARITY_HIGH
#define TIM_BreakPolarity_Low           TIM_BREAKPOLARITY_LOW
#define TIM_AutomaticOutput_Enable      TIM_AUTOMATICOUTPUT_ENABLE
#define TIM_AutomaticOutput_Disable     TIM_AUTOMATICOUTPUT_DISABLE

// Timer Channel
#define TIM_Channel_1                   TIM_CHANNEL_1
#define TIM_Channel_2                   TIM_CHANNEL_2
#define TIM_Channel_3                   TIM_CHANNEL_3
#define TIM_Channel_4                   TIM_CHANNEL_4

// Timer CCx
#define TIM_CCx_Enable                  TIM_CCx_ENABLE
#define TIM_CCx_Disable                 TIM_CCx_DISABLE
#define TIM_CCxN_Enable                 TIM_CCxN_ENABLE
#define TIM_CCxN_Disable                TIM_CCxN_DISABLE

// Timer Forced Action
#define TIM_ForcedAction_Active         TIM_OCMODE_FORCED_ACTIVE
#define TIM_ForcedAction_InActive       TIM_OCMODE_FORCED_INACTIVE

// Timer TRGO Source
#define TIM_TRGOSource_Update           TIM_TRGO_UPDATE
#define TIM_TRGOSource_Enable           TIM_TRGO_ENABLE

// Timer Master Slave Mode
#define TIM_MasterSlaveMode_Enable      TIM_MASTERSLAVEMODE_ENABLE

// Timer Slave Mode
#define TIM_SlaveMode_Reset             TIM_SLAVEMODE_RESET
#define TIM_SlaveMode_Trigger           TIM_SLAVEMODE_TRIGGER

// Timer Trigger Selection (Input Trigger Source)
#undef TIM_TS_ITR0
#define TIM_TS_ITR0                 0x0000
#undef TIM_TS_ITR1
#define TIM_TS_ITR1                 0x0010
#undef TIM_TS_ITR2
#define TIM_TS_ITR2                 0x0020
#undef TIM_TS_ITR3
#define TIM_TS_ITR3                 0x0030

// Timer Event Source
#define TIM_EventSource_COM             TIM_EVENTSOURCE_COM
#define TIM_EventSource_Update          TIM_EVENTSOURCE_UPDATE

// Timer IT
#undef TIM_IT_UPDATE
#define TIM_IT_UPDATE                   0x0001
#undef TIM_IT_CC1
#define TIM_IT_CC1                      0x0002
#undef TIM_IT_CC2
#define TIM_IT_CC2                      0x0004
#undef TIM_IT_CC3
#define TIM_IT_CC3                      0x0008
#undef TIM_IT_CC4
#define TIM_IT_CC4                      0x0010
#define TIM_IT_Update                   TIM_IT_UPDATE

// Timer OC Preload
#define TIM_OCPRELOAD_ENABLE            1
#define TIM_OCPreload_Enable            TIM_OCPRELOAD_ENABLE

// Timer Init Structures
typedef struct {
    uint16_t TIM_Prescaler;
    uint16_t TIM_CounterMode;
    uint32_t TIM_Period;
    uint16_t TIM_ClockDivision;
    uint8_t TIM_RepetitionCounter;
} TIM_TimeBaseInitTypeDef;

// Custom TIM_OCInitTypeDef for F4 compatibility (F1 HAL structure is different)
typedef struct {
    uint32_t TIM_OCMode;
    uint32_t TIM_OutputState;
    uint32_t TIM_OutputNState;
    uint32_t TIM_Pulse;
    uint32_t TIM_OCPolarity;
    uint32_t TIM_OCNPolarity;
    uint32_t TIM_OCIdleState;
    uint32_t TIM_OCNIdleState;
} TIM_OCInitTypeDef;

// Custom TIM_BDTRInitTypeDef for F4 compatibility
typedef struct {
    uint32_t TIM_OSSRState;
    uint32_t TIM_OSSIState;
    uint32_t TIM_LockLevel;
    uint32_t TIM_DeadTime;
    uint32_t TIM_Break;
    uint32_t TIM_BreakPolarity;
    uint32_t TIM_AutomaticOutput;
} TIM_BDTRInitTypeDef;

// Timer Functions (wrappers for HAL)
void TIM_TimeBaseInit(TIM_TypeDef* TIMx, TIM_TimeBaseInitTypeDef* TIM_TimeBaseInitStruct);
void TIM_OC1Init(TIM_TypeDef* TIMx, TIM_OCInitTypeDef* TIM_OCInitStruct);
void TIM_OC2Init(TIM_TypeDef* TIMx, TIM_OCInitTypeDef* TIM_OCInitStruct);
void TIM_OC3Init(TIM_TypeDef* TIMx, TIM_OCInitTypeDef* TIM_OCInitStruct);
void TIM_OC4Init(TIM_TypeDef* TIMx, TIM_OCInitTypeDef* TIM_OCInitStruct);
void TIM_OC1PreloadConfig(TIM_TypeDef* TIMx, uint16_t TIM_OCPreload);
void TIM_OC2PreloadConfig(TIM_TypeDef* TIMx, uint16_t TIM_OCPreload);
void TIM_OC3PreloadConfig(TIM_TypeDef* TIMx, uint16_t TIM_OCPreload);
void TIM_OC4PreloadConfig(TIM_TypeDef* TIMx, uint16_t TIM_OCPreload);
void TIM_BDTRConfig(TIM_TypeDef* TIMx, TIM_BDTRInitTypeDef* TIM_BDTRInitStruct);
void TIM_CCPreloadControl(TIM_TypeDef* TIMx, FunctionalState NewState);
void TIM_ARRPreloadConfig(TIM_TypeDef* TIMx, FunctionalState NewState);
void TIM_CtrlPWMOutputs(TIM_TypeDef* TIMx, FunctionalState NewState);
void TIM_SelectOutputTrigger(TIM_TypeDef* TIMx, uint16_t TIM_TRGOSource);
void TIM_SelectMasterSlaveMode(TIM_TypeDef* TIMx, uint16_t TIM_MasterSlaveMode);
void TIM_SelectInputTrigger(TIM_TypeDef* TIMx, uint16_t TIM_InputTriggerSource);
void TIM_SelectSlaveMode(TIM_TypeDef* TIMx, uint16_t TIM_SlaveMode);
void TIM_Cmd(TIM_TypeDef* TIMx, FunctionalState NewState);
void TIM_ITConfig(TIM_TypeDef* TIMx, uint16_t TIM_IT, FunctionalState NewState);
void TIM_GenerateEvent(TIM_TypeDef* TIMx, uint16_t TIM_EventSource);
void TIM_SelectOCxM(TIM_TypeDef* TIMx, uint16_t TIM_Channel, uint16_t TIM_OCMode);
void TIM_CCxCmd(TIM_TypeDef* TIMx, uint16_t TIM_Channel, uint16_t TIM_CCx);
void TIM_CCxNCmd(TIM_TypeDef* TIMx, uint16_t TIM_Channel, uint16_t TIM_CCxN);
void TIM_DeInit(TIM_TypeDef* TIMx);

#ifdef __cplusplus
}
#endif

#endif /* STM32F4_COMPAT_H */
