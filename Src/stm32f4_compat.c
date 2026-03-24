/*
 * STM32F4 to STM32F1 Compatibility Layer Implementation
 */

#include "stm32f4_compat.h"
#include <string.h>

/*============================================================================
 * ADC Common Register Emulation
 *============================================================================*/

// Create dummy ADC common structure for F1
static ADC_Common_TypeDef_F4 ADC_Common = {0};
ADC_Common_TypeDef_F4* ADC = &ADC_Common;

/*============================================================================
 * DMA Functions
 *============================================================================*/

void DMA_Init(DMA_Channel_TypeDef* DMAy_Channelx, DMA_InitTypeDef* DMA_InitStruct) {
    uint32_t tmpreg = 0;
    
    // Disable DMA channel
    DMAy_Channelx->CCR &= ~DMA_CCR_EN;
    
    // Configure DMA channel
    tmpreg = DMAy_Channelx->CCR;
    tmpreg &= ~(DMA_CCR_DIR | DMA_CCR_CIRC | DMA_CCR_PINC | DMA_CCR_MINC | 
                DMA_CCR_PSIZE | DMA_CCR_MSIZE | DMA_CCR_PL);
    
    // Direction
    if (DMA_InitStruct->DMA_DIR == DMA_PERIPH_TO_MEMORY) {
        tmpreg &= ~DMA_CCR_DIR;
    } else {
        tmpreg |= DMA_CCR_DIR;
    }
    
    // Circular mode
    if (DMA_InitStruct->DMA_Mode == DMA_CIRCULAR) {
        tmpreg |= DMA_CCR_CIRC;
    }
    
    // Peripheral increment
    if (DMA_InitStruct->DMA_PeripheralInc == DMA_PINC_ENABLE) {
        tmpreg |= DMA_CCR_PINC;
    }
    
    // Memory increment
    if (DMA_InitStruct->DMA_MemoryInc == DMA_MINC_ENABLE) {
        tmpreg |= DMA_CCR_MINC;
    }
    
    // Data sizes
    tmpreg |= DMA_InitStruct->DMA_PeripheralDataSize;
    tmpreg |= DMA_InitStruct->DMA_MemoryDataSize;
    
    // Priority
    tmpreg |= DMA_InitStruct->DMA_Priority;
    
    DMAy_Channelx->CCR = tmpreg;
    
    // Set addresses and buffer size
    DMAy_Channelx->CPAR = DMA_InitStruct->DMA_PeripheralBaseAddr;
    DMAy_Channelx->CMAR = DMA_InitStruct->DMA_Memory0BaseAddr;
    DMAy_Channelx->CNDTR = DMA_InitStruct->DMA_BufferSize;
}

void DMA_Cmd(DMA_Channel_TypeDef* DMAy_Channelx, FunctionalState NewState) {
    if (NewState != DISABLE) {
        DMAy_Channelx->CCR |= DMA_CCR_EN;
    } else {
        DMAy_Channelx->CCR &= ~DMA_CCR_EN;
    }
}

void DMA_ITConfig(DMA_Channel_TypeDef* DMAy_Channelx, uint32_t DMA_IT, FunctionalState NewState) {
    if (NewState != DISABLE) {
        // DMA_IT is a bitmask, enable the corresponding interrupt bits
        if (DMA_IT & 0x0002) {  // TC interrupt
            DMAy_Channelx->CCR |= DMA_CCR_TCIE;
        }
        if (DMA_IT & 0x0004) {  // HT interrupt
            DMAy_Channelx->CCR |= DMA_CCR_HTIE;
        }
    } else {
        if (DMA_IT & 0x0002) {  // TC interrupt
            DMAy_Channelx->CCR &= ~DMA_CCR_TCIE;
        }
        if (DMA_IT & 0x0004) {  // HT interrupt
            DMAy_Channelx->CCR &= ~DMA_CCR_HTIE;
        }
    }
}

void DMA_DeInit(DMA_Channel_TypeDef* DMAy_Channelx) {
    DMAy_Channelx->CCR &= ~DMA_CCR_EN;
    DMAy_Channelx->CCR = 0;
    DMAy_Channelx->CNDTR = 0;
    DMAy_Channelx->CPAR = 0;
    DMAy_Channelx->CMAR = 0;
    
    // Clear all flags
    if (DMAy_Channelx == DMA1_Channel1) {
        DMA1->IFCR = DMA_IFCR_CGIF1 | DMA_IFCR_CTCIF1 | DMA_IFCR_CHTIF1 | DMA_IFCR_CTEIF1;
    } else if (DMAy_Channelx == DMA2_Channel5) {
        DMA2->IFCR = DMA_IFCR_CGIF5 | DMA_IFCR_CTCIF5 | DMA_IFCR_CHTIF5 | DMA_IFCR_CTEIF5;
    }
}

/*============================================================================
 * ADC Functions
 *============================================================================*/

void ADC_CommonInit(ADC_CommonInitTypeDef_F4* ADC_CommonInitStruct) {
    // F1 doesn't have common ADC configuration
    // Set ADC prescaler in RCC
    uint32_t tmpreg = RCC->CFGR;
    tmpreg &= ~RCC_CFGR_ADCPRE;
    tmpreg |= ADC_CommonInitStruct->ADC_Prescaler;
    RCC->CFGR = tmpreg;
    
    // Enable dual mode if needed
    if (ADC_CommonInitStruct->ADC_Mode == ADC_DUALMODE_REGSIMULT) {
        ADC1->CR1 |= ADC_CR1_DUALMOD_2 | ADC_CR1_DUALMOD_1; // Dual regular simultaneous mode
    }
}

void ADC_Init(ADC_TypeDef* ADCx, ADC_InitTypeDef_F4* ADC_InitStruct) {
    uint32_t tmpreg1 = 0;
    
    // CR1 configuration
    tmpreg1 = ADCx->CR1;
    tmpreg1 &= ~(ADC_CR1_SCAN);
    
    if (ADC_InitStruct->ADC_ScanConvMode == ENABLE) {
        tmpreg1 |= ADC_CR1_SCAN;
    }
    
    ADCx->CR1 = tmpreg1;
    
    // CR2 configuration
    tmpreg1 = ADCx->CR2;
    tmpreg1 &= ~(ADC_CR2_CONT | ADC_CR2_ALIGN | ADC_CR2_EXTSEL | ADC_CR2_EXTTRIG);
    
    if (ADC_InitStruct->ADC_ContinuousConvMode == ENABLE) {
        tmpreg1 |= ADC_CR2_CONT;
    }
    
    tmpreg1 |= ADC_InitStruct->ADC_DataAlign;
    tmpreg1 |= ADC_InitStruct->ADC_ExternalTrigConv;
    
    if (ADC_InitStruct->ADC_ExternalTrigConv != 0) {
        tmpreg1 |= ADC_CR2_EXTTRIG;
    }
    
    ADCx->CR2 = tmpreg1;
    
    // SQR1 - Number of conversions
    ADCx->SQR1 &= ~ADC_SQR1_L;
    ADCx->SQR1 |= (uint32_t)((ADC_InitStruct->ADC_NbrOfConversion - 1) << 20);
}

void ADC_Cmd(ADC_TypeDef* ADCx, FunctionalState NewState) {
    if (NewState != DISABLE) {
        ADCx->CR2 |= ADC_CR2_ADON;
        
        // Wait for ADC to stabilize
        for (volatile int i = 0; i < 1000; i++);
        
        // Calibrate ADC
        ADCx->CR2 |= ADC_CR2_CAL;
        while (ADCx->CR2 & ADC_CR2_CAL);
    } else {
        ADCx->CR2 &= ~ADC_CR2_ADON;
    }
}

void ADC_DeInit(void) {
    RCC->APB2RSTR |= RCC_APB2RSTR_ADC1RST | RCC_APB2RSTR_ADC2RST;
    RCC->APB2RSTR &= ~(RCC_APB2RSTR_ADC1RST | RCC_APB2RSTR_ADC2RST);
}

void ADC_TempSensorVrefintCmd(FunctionalState NewState) {
    if (NewState != DISABLE) {
        ADC1->CR2 |= ADC_CR2_TSVREFE;
    } else {
        ADC1->CR2 &= ~ADC_CR2_TSVREFE;
    }
}

void ADC_MultiModeDMARequestAfterLastTransferCmd(FunctionalState NewState) {
    // F1 dual mode DMA is always enabled when dual mode is active
    (void)NewState;
}

void ADC_ExternalTrigInjectedConvConfig(ADC_TypeDef* ADCx, uint32_t ADC_ExternalTrigInjecConv) {
    uint32_t tmpreg = ADCx->CR2;
    tmpreg &= ~ADC_CR2_JEXTSEL;
    tmpreg |= ADC_ExternalTrigInjecConv;
    ADCx->CR2 = tmpreg;
}

void ADC_ExternalTrigInjectedConvEdgeConfig(ADC_TypeDef* ADCx, uint32_t ADC_ExternalTrigInjecConvEdge) {
    if (ADC_ExternalTrigInjecConvEdge != 0) {
        ADCx->CR2 |= ADC_CR2_JEXTTRIG;
    } else {
        ADCx->CR2 &= ~ADC_CR2_JEXTTRIG;
    }
}

void ADC_InjectedSequencerLengthConfig(ADC_TypeDef* ADCx, uint8_t Length) {
    uint32_t tmpreg = ADCx->JSQR;
    tmpreg &= ~ADC_JSQR_JL;
    tmpreg |= (uint32_t)((Length - 1) << 20);
    ADCx->JSQR = tmpreg;
}

void ADC_ITConfig(ADC_TypeDef* ADCx, uint16_t ADC_IT, FunctionalState NewState) {
    if (NewState != DISABLE) {
        ADCx->CR1 |= ADC_IT;
    } else {
        ADCx->CR1 &= ~ADC_IT;
    }
}

/*============================================================================
 * TIM Functions
 *============================================================================*/

void TIM_TimeBaseInit(TIM_TypeDef* TIMx, TIM_TimeBaseInitTypeDef* TIM_TimeBaseInitStruct) {
    TIM_HandleTypeDef htim = {0};
    htim.Instance = TIMx;
    htim.Init.Prescaler = TIM_TimeBaseInitStruct->TIM_Prescaler;
    htim.Init.CounterMode = TIM_TimeBaseInitStruct->TIM_CounterMode;
    htim.Init.Period = TIM_TimeBaseInitStruct->TIM_Period;
    htim.Init.ClockDivision = TIM_TimeBaseInitStruct->TIM_ClockDivision;
    htim.Init.RepetitionCounter = TIM_TimeBaseInitStruct->TIM_RepetitionCounter;
    HAL_TIM_Base_Init(&htim);
}

void TIM_OC1Init(TIM_TypeDef* TIMx, TIM_OCInitTypeDef* TIM_OCInitStruct) {
    TIMx->CCR1 = TIM_OCInitStruct->TIM_Pulse;
    
    uint32_t tmpccmrx = TIMx->CCMR1;
    tmpccmrx &= ~(TIM_CCMR1_OC1M | TIM_CCMR1_CC1S);
    tmpccmrx |= (TIM_OCInitStruct->TIM_OCMode & 0x07) << 4;
    TIMx->CCMR1 = tmpccmrx;
    
    uint32_t tmpccer = TIMx->CCER;
    tmpccer &= ~(TIM_CCER_CC1E | TIM_CCER_CC1P | TIM_CCER_CC1NE | TIM_CCER_CC1NP);
    
    // OCPolarity
    if (TIM_OCInitStruct->TIM_OCPolarity == TIM_OCPOLARITY_LOW) {
        tmpccer |= TIM_CCER_CC1P;
    }
    // OCNPolarity
    if (TIM_OCInitStruct->TIM_OCNPolarity == TIM_OCNPOLARITY_LOW) {
        tmpccer |= TIM_CCER_CC1NP;
    }
    // Always enable outputs (controlled by OutputState/OutputNState in F4)
    tmpccer |= TIM_CCER_CC1E | TIM_CCER_CC1NE;
    
    TIMx->CCER = tmpccer;
}

void TIM_OC2Init(TIM_TypeDef* TIMx, TIM_OCInitTypeDef* TIM_OCInitStruct) {
    TIMx->CCR2 = TIM_OCInitStruct->TIM_Pulse;
    
    uint32_t tmpccmrx = TIMx->CCMR1;
    tmpccmrx &= ~(TIM_CCMR1_OC2M | TIM_CCMR1_CC2S);
    tmpccmrx |= ((TIM_OCInitStruct->TIM_OCMode & 0x07) << 12);
    TIMx->CCMR1 = tmpccmrx;
    
    uint32_t tmpccer = TIMx->CCER;
    tmpccer &= ~(TIM_CCER_CC2E | TIM_CCER_CC2P | TIM_CCER_CC2NE | TIM_CCER_CC2NP);
    
    if (TIM_OCInitStruct->TIM_OCPolarity == TIM_OCPOLARITY_LOW) {
        tmpccer |= TIM_CCER_CC2P;
    }
    if (TIM_OCInitStruct->TIM_OCNPolarity == TIM_OCNPOLARITY_LOW) {
        tmpccer |= TIM_CCER_CC2NP;
    }
    tmpccer |= TIM_CCER_CC2E | TIM_CCER_CC2NE;
    TIMx->CCER = tmpccer;
}

void TIM_OC3Init(TIM_TypeDef* TIMx, TIM_OCInitTypeDef* TIM_OCInitStruct) {
    TIMx->CCR3 = TIM_OCInitStruct->TIM_Pulse;
    
    uint32_t tmpccmrx = TIMx->CCMR2;
    tmpccmrx &= ~(TIM_CCMR2_OC3M | TIM_CCMR2_CC3S);
    tmpccmrx |= (TIM_OCInitStruct->TIM_OCMode & 0x07) << 4;
    TIMx->CCMR2 = tmpccmrx;
    
    uint32_t tmpccer = TIMx->CCER;
    tmpccer &= ~(TIM_CCER_CC3E | TIM_CCER_CC3P | TIM_CCER_CC3NE | TIM_CCER_CC3NP);
    
    if (TIM_OCInitStruct->TIM_OCPolarity == TIM_OCPOLARITY_LOW) {
        tmpccer |= TIM_CCER_CC3P;
    }
    if (TIM_OCInitStruct->TIM_OCNPolarity == TIM_OCNPOLARITY_LOW) {
        tmpccer |= TIM_CCER_CC3NP;
    }
    tmpccer |= TIM_CCER_CC3E | TIM_CCER_CC3NE;
    TIMx->CCER = tmpccer;
}

void TIM_OC4Init(TIM_TypeDef* TIMx, TIM_OCInitTypeDef* TIM_OCInitStruct) {
    TIMx->CCR4 = TIM_OCInitStruct->TIM_Pulse;
    
    uint32_t tmpccmrx = TIMx->CCMR2;
    tmpccmrx &= ~(TIM_CCMR2_OC4M | TIM_CCMR2_CC4S);
    tmpccmrx |= ((TIM_OCInitStruct->TIM_OCMode & 0x07) << 12);
    TIMx->CCMR2 = tmpccmrx;
    
    uint32_t tmpccer = TIMx->CCER;
    tmpccer &= ~(TIM_CCER_CC4E | TIM_CCER_CC4P);
    
    if (TIM_OCInitStruct->TIM_OCPolarity == TIM_OCPOLARITY_LOW) {
        tmpccer |= TIM_CCER_CC4P;
    }
    tmpccer |= TIM_CCER_CC4E;
    TIMx->CCER = tmpccer;
}

void TIM_OC1PreloadConfig(TIM_TypeDef* TIMx, uint16_t TIM_OCPreload) {
    if (TIM_OCPreload != 0) {
        TIMx->CCMR1 |= TIM_CCMR1_OC1PE;
    } else {
        TIMx->CCMR1 &= ~TIM_CCMR1_OC1PE;
    }
}

void TIM_OC2PreloadConfig(TIM_TypeDef* TIMx, uint16_t TIM_OCPreload) {
    if (TIM_OCPreload != 0) {
        TIMx->CCMR1 |= TIM_CCMR1_OC2PE;
    } else {
        TIMx->CCMR1 &= ~TIM_CCMR1_OC2PE;
    }
}

void TIM_OC3PreloadConfig(TIM_TypeDef* TIMx, uint16_t TIM_OCPreload) {
    if (TIM_OCPreload != 0) {
        TIMx->CCMR2 |= TIM_CCMR2_OC3PE;
    } else {
        TIMx->CCMR2 &= ~TIM_CCMR2_OC3PE;
    }
}

void TIM_OC4PreloadConfig(TIM_TypeDef* TIMx, uint16_t TIM_OCPreload) {
    if (TIM_OCPreload != 0) {
        TIMx->CCMR2 |= TIM_CCMR2_OC4PE;
    } else {
        TIMx->CCMR2 &= ~TIM_CCMR2_OC4PE;
    }
}

void TIM_BDTRConfig(TIM_TypeDef* TIMx, TIM_BDTRInitTypeDef *TIM_BDTRInitStruct) {
    uint32_t tmpbdtr = 0;
    
    tmpbdtr = TIMx->BDTR;
    tmpbdtr &= ~(TIM_BDTR_DTG | TIM_BDTR_LOCK | TIM_BDTR_OSSI | TIM_BDTR_OSSR | 
                 TIM_BDTR_BKE | TIM_BDTR_BKP | TIM_BDTR_AOE);
    
    tmpbdtr |= TIM_BDTRInitStruct->TIM_DeadTime;
    tmpbdtr |= TIM_BDTRInitStruct->TIM_LockLevel;
    tmpbdtr |= TIM_BDTRInitStruct->TIM_OSSIState;
    tmpbdtr |= TIM_BDTRInitStruct->TIM_OSSRState;
    tmpbdtr |= TIM_BDTRInitStruct->TIM_Break;
    tmpbdtr |= TIM_BDTRInitStruct->TIM_BreakPolarity;
    tmpbdtr |= TIM_BDTRInitStruct->TIM_AutomaticOutput;
    
    TIMx->BDTR = tmpbdtr;
}

void TIM_CCPreloadControl(TIM_TypeDef* TIMx, FunctionalState NewState) {
    if (NewState != DISABLE) {
        TIMx->CR2 |= TIM_CR2_CCPC;
    } else {
        TIMx->CR2 &= ~TIM_CR2_CCPC;
    }
}

void TIM_ARRPreloadConfig(TIM_TypeDef* TIMx, FunctionalState NewState) {
    if (NewState != DISABLE) {
        TIMx->CR1 |= TIM_CR1_ARPE;
    } else {
        TIMx->CR1 &= ~TIM_CR1_ARPE;
    }
}

void TIM_CtrlPWMOutputs(TIM_TypeDef* TIMx, FunctionalState NewState) {
    if (NewState != DISABLE) {
        TIMx->BDTR |= TIM_BDTR_MOE;
    } else {
        TIMx->BDTR &= ~TIM_BDTR_MOE;
    }
}

void TIM_SelectOutputTrigger(TIM_TypeDef* TIMx, uint16_t TIM_TRGOSource) {
    TIMx->CR2 &= ~TIM_CR2_MMS;
    TIMx->CR2 |= TIM_TRGOSource;
}

void TIM_SelectMasterSlaveMode(TIM_TypeDef* TIMx, uint16_t TIM_MasterSlaveMode) {
    if (TIM_MasterSlaveMode == TIM_MASTERSLAVEMODE_ENABLE) {
        TIMx->SMCR |= TIM_SMCR_MSM;
    } else {
        TIMx->SMCR &= ~TIM_SMCR_MSM;
    }
}

void TIM_SelectInputTrigger(TIM_TypeDef* TIMx, uint16_t TIM_InputTriggerSource) {
    TIMx->SMCR &= ~TIM_SMCR_TS;
    TIMx->SMCR |= TIM_InputTriggerSource;
}

void TIM_SelectSlaveMode(TIM_TypeDef* TIMx, uint16_t TIM_SlaveMode) {
    TIMx->SMCR &= ~TIM_SMCR_SMS;
    TIMx->SMCR |= TIM_SlaveMode;
}

void TIM_Cmd(TIM_TypeDef* TIMx, FunctionalState NewState) {
    if (NewState != DISABLE) {
        TIMx->CR1 |= TIM_CR1_CEN;
    } else {
        TIMx->CR1 &= ~TIM_CR1_CEN;
    }
}

void TIM_ITConfig(TIM_TypeDef* TIMx, uint16_t TIM_IT, FunctionalState NewState) {
    if (NewState != DISABLE) {
        TIMx->DIER |= TIM_IT;
    } else {
        TIMx->DIER &= ~TIM_IT;
    }
}

void TIM_GenerateEvent(TIM_TypeDef* TIMx, uint16_t TIM_EventSource) {
    TIMx->EGR = TIM_EventSource;
}

void TIM_SelectOCxM(TIM_TypeDef* TIMx, uint16_t TIM_Channel, uint16_t TIM_OCMode) {
    if (TIM_Channel == TIM_CHANNEL_1) {
        TIMx->CCMR1 &= ~TIM_CCMR1_OC1M;
        TIMx->CCMR1 |= TIM_OCMode;
    } else if (TIM_Channel == TIM_CHANNEL_2) {
        TIMx->CCMR1 &= ~TIM_CCMR1_OC2M;
        TIMx->CCMR1 |= (TIM_OCMode << 8);
    } else if (TIM_Channel == TIM_CHANNEL_3) {
        TIMx->CCMR2 &= ~TIM_CCMR2_OC3M;
        TIMx->CCMR2 |= TIM_OCMode;
    } else if (TIM_Channel == TIM_CHANNEL_4) {
        TIMx->CCMR2 &= ~TIM_CCMR2_OC4M;
        TIMx->CCMR2 |= (TIM_OCMode << 8);
    }
}

void TIM_CCxCmd(TIM_TypeDef* TIMx, uint16_t TIM_Channel, uint16_t TIM_CCx) {
    uint32_t tmp = 0;
    
    if (TIM_Channel == TIM_CHANNEL_1) {
        tmp = TIM_CCER_CC1E;
    } else if (TIM_Channel == TIM_CHANNEL_2) {
        tmp = TIM_CCER_CC2E;
    } else if (TIM_Channel == TIM_CHANNEL_3) {
        tmp = TIM_CCER_CC3E;
    } else if (TIM_Channel == TIM_CHANNEL_4) {
        tmp = TIM_CCER_CC4E;
    }
    
    if (TIM_CCx == TIM_CCx_ENABLE) {
        TIMx->CCER |= tmp;
    } else {
        TIMx->CCER &= ~tmp;
    }
}

void TIM_CCxNCmd(TIM_TypeDef* TIMx, uint16_t TIM_Channel, uint16_t TIM_CCxN) {
    uint32_t tmp = 0;
    
    if (TIM_Channel == TIM_CHANNEL_1) {
        tmp = TIM_CCER_CC1NE;
    } else if (TIM_Channel == TIM_CHANNEL_2) {
        tmp = TIM_CCER_CC2NE;
    } else if (TIM_Channel == TIM_CHANNEL_3) {
        tmp = TIM_CCER_CC3NE;
    }
    
    if (TIM_CCxN == TIM_CCxN_ENABLE) {
        TIMx->CCER |= tmp;
    } else {
        TIMx->CCER &= ~tmp;
    }
}

void TIM_DeInit(TIM_TypeDef* TIMx) {
    if (TIMx == TIM1) {
        RCC->APB2RSTR |= RCC_APB2RSTR_TIM1RST;
        RCC->APB2RSTR &= ~RCC_APB2RSTR_TIM1RST;
    } else if (TIMx == TIM2) {
        RCC->APB1RSTR |= RCC_APB1RSTR_TIM2RST;
        RCC->APB1RSTR &= ~RCC_APB1RSTR_TIM2RST;
    } else if (TIMx == TIM3) {
        RCC->APB1RSTR |= RCC_APB1RSTR_TIM3RST;
        RCC->APB1RSTR &= ~RCC_APB1RSTR_TIM3RST;
    } else if (TIMx == TIM4) {
        RCC->APB1RSTR |= RCC_APB1RSTR_TIM4RST;
        RCC->APB1RSTR &= ~RCC_APB1RSTR_TIM4RST;
    } else if (TIMx == TIM8) {
        RCC->APB2RSTR |= RCC_APB2RSTR_TIM8RST;
        RCC->APB2RSTR &= ~RCC_APB2RSTR_TIM8RST;
    }
}
