#include "hal_gpio.h"

static void enable_port_clock(GPIO_TypeDef *port) {
	if (port == GPIOA) __HAL_RCC_GPIOA_CLK_ENABLE();
	else if (port == GPIOB) __HAL_RCC_GPIOB_CLK_ENABLE();
	else if (port == GPIOC) __HAL_RCC_GPIOC_CLK_ENABLE();
	else if (port == GPIOD) __HAL_RCC_GPIOD_CLK_ENABLE();
	else if (port == GPIOE) __HAL_RCC_GPIOE_CLK_ENABLE();
	else if (port == GPIOF) __HAL_RCC_GPIOF_CLK_ENABLE();
}

void hal_gpio_init_output(GPIO_TypeDef *port, uint16_t pin) {
	enable_port_clock(port);

	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(port, &GPIO_InitStruct);
}

void hal_gpio_init_output_od(GPIO_TypeDef *port, uint16_t pin) {
	enable_port_clock(port);

	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(port, &GPIO_InitStruct);
}

void hal_gpio_init_input(GPIO_TypeDef *port, uint16_t pin) {
	enable_port_clock(port);

	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(port, &GPIO_InitStruct);
}

void hal_gpio_init_input_pullup(GPIO_TypeDef *port, uint16_t pin) {
	enable_port_clock(port);

	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(port, &GPIO_InitStruct);
}

void hal_gpio_init_input_pulldown(GPIO_TypeDef *port, uint16_t pin) {
	enable_port_clock(port);

	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(port, &GPIO_InitStruct);
}

void hal_gpio_init_input_analog(GPIO_TypeDef *port, uint16_t pin) {
	enable_port_clock(port);

	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = pin;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(port, &GPIO_InitStruct);
}

void hal_gpio_init_af(GPIO_TypeDef *port, uint16_t pin, uint8_t af) {
	enable_port_clock(port);

	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(port, &GPIO_InitStruct);
	// Note: STM32F1 does not have per-pin AF selection via GPIO registers.
	// AF remapping is done via AFIO peripheral (AFIO_MAPR) separately.
	(void)af;
}

void hal_gpio_write(GPIO_TypeDef *port, uint16_t pin, bool state) {
	HAL_GPIO_WritePin(port, pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

bool hal_gpio_read(GPIO_TypeDef *port, uint16_t pin) {
	return HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_SET;
}

void hal_gpio_toggle(GPIO_TypeDef *port, uint16_t pin) {
	HAL_GPIO_TogglePin(port, pin);
}

void hal_gpio_set(GPIO_TypeDef *port, uint16_t pin) {
	HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
}

void hal_gpio_clear(GPIO_TypeDef *port, uint16_t pin) {
	HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
}
