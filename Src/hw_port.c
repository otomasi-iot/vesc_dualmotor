// Hoverboard port glue for VESC-style entrypoints expected by Src/main.c.

#include "hw_config.h"

// Provided by Src/hw_setup.c
void MX_GPIO_Init(void);

void hw_init_gpio(void) {
	MX_GPIO_Init();
}

