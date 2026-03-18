#include "stm32f1xx_hal.h"
#include "cmsis_os2.h"

SPI_HandleTypeDef hspi1;

void spi_hw_init(void) {
    // Enable SPI1 clock
    __HAL_RCC_SPI1_CLK_ENABLE();

    // SPI configuration
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;  // CPOL=0
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;      // CPHA=0
    hspi1.Init.NSS = SPI_NSS_SOFT;              // Software chip select      
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;  // 72MHz/16 = 4.5 MHz
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;

    HAL_SPI_Init(&hspi1);
}

void spi_hw_write_read(const uint8_t *tx, uint8_t *rx, int len) {
    // Pull CS low
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);

    // SPI transaction
    HAL_SPI_TransmitReceive(&hspi1, (uint8_t *)tx, rx, len, 1000);

    // Pull CS high
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
}
