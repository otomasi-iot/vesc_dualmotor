---
name: HAL GPIO Migration Agent
description: |
  Specialized agent for converting ChibiOS PAL GPIO abstraction to STM32F1 HAL GPIO layer.
  Use when: Converting palReadPad(), palWritePad(), palSetPadMode() calls to HAL_GPIO_* equivalents.
  Handles: GPIO pin abstractions, board configuration, hal.h adaptation layer.
  Scope: Src/hwconf/, GPIO-related patterns across all drivers and applications.
---

# HAL GPIO & Hardware Configuration Migration Agent

## Mission
Convert ChibiOS PAL (Platform Abstraction Layer) GPIO calls to STM32F1 HAL GPIO driver while maintaining:
- Exact pin mappings from hwconf board configs
- GPIO mode consistency (input/output, pushpull/opendrain, pullup/pulldown)
- ISR callback structures for external interrupts
- Dual motor synchronized gate driver outputs

## FASE 1: KONVERSI GPIO - NO WRAPPER, DIRECT REPLACEMENT

**PENTING**: Tidak ada wrapper, tidak ada abstraksi. Setiap `palReadPad`, `palWritePad`, `palSetPadMode` diganti langsung dengan call HAL atau akses register.

---

## LANGKAH 1: Buat hal_gpio.h (FILE BARU)

**Lokasi**: `Src/hwconf/hal_gpio.h`

**Isi file (COPAS TEPAT INI)**:
```c
#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

// Direct register macros untuk ISR motor (NO FUNCTION CALL OVERHEAD)
#define GPIO_PIN_SET_DIRECT(port, pin)    do { (port)->BSRR = (1 << (pin)); } while(0)
#define GPIO_PIN_RESET_DIRECT(port, pin)  do { (port)->BRR = (1 << (pin)); } while(0)
#define GPIO_PIN_READ_DIRECT(port, pin)   (((port)->IDR >> (pin)) & 1)

// Standard HAL function calls untuk non-ISR
void hal_gpio_init_output(GPIO_TypeDef *port, uint16_t pin);
void hal_gpio_init_input(GPIO_TypeDef *port, uint16_t pin);
void hal_gpio_init_input_pullup(GPIO_TypeDef *port, uint16_t pin);
void hal_gpio_init_input_pulldown(GPIO_TypeDef *port, uint16_t pin);

void hal_gpio_write(GPIO_TypeDef *port, uint16_t pin, bool state);
bool hal_gpio_read(GPIO_TypeDef *port, uint16_t pin);
void hal_gpio_toggle(GPIO_TypeDef *port, uint16_t pin);

#endif
```

---

## LANGKAH 2: Buat hal_gpio.c (FILE BARU)

**Lokasi**: `Src/hwconf/hal_gpio.c`

**Isi file (COPAS INI)**:
```c
#include "hal_gpio.h"

void hal_gpio_init_output(GPIO_TypeDef *port, uint16_t pin) {
    // Enable clock untuk port
    if (port == GPIOA) __HAL_RCC_GPIOA_CLK_ENABLE();
    else if (port == GPIOB) __HAL_RCC_GPIOB_CLK_ENABLE();
    else if (port == GPIOC) __HAL_RCC_GPIOC_CLK_ENABLE();
    else if (port == GPIOD) __HAL_RCC_GPIOD_CLK_ENABLE();
    else if (port == GPIOE) __HAL_RCC_GPIOE_CLK_ENABLE();
    else if (port == GPIOF) __HAL_RCC_GPIOF_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(port, &GPIO_InitStruct);
}

void hal_gpio_init_input(GPIO_TypeDef *port, uint16_t pin) {
    if (port == GPIOA) __HAL_RCC_GPIOA_CLK_ENABLE();
    else if (port == GPIOB) __HAL_RCC_GPIOB_CLK_ENABLE();
    else if (port == GPIOC) __HAL_RCC_GPIOC_CLK_ENABLE();
    else if (port == GPIOD) __HAL_RCC_GPIOD_CLK_ENABLE();
    else if (port == GPIOE) __HAL_RCC_GPIOE_CLK_ENABLE();
    else if (port == GPIOF) __HAL_RCC_GPIOF_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(port, &GPIO_InitStruct);
}

void hal_gpio_init_input_pullup(GPIO_TypeDef *port, uint16_t pin) {
    if (port == GPIOA) __HAL_RCC_GPIOA_CLK_ENABLE();
    else if (port == GPIOB) __HAL_RCC_GPIOB_CLK_ENABLE();
    else if (port == GPIOC) __HAL_RCC_GPIOC_CLK_ENABLE();
    else if (port == GPIOD) __HAL_RCC_GPIOD_CLK_ENABLE();
    else if (port == GPIOE) __HAL_RCC_GPIOE_CLK_ENABLE();
    else if (port == GPIOF) __HAL_RCC_GPIOF_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(port, &GPIO_InitStruct);
}

void hal_gpio_init_input_pulldown(GPIO_TypeDef *port, uint16_t pin) {
    if (port == GPIOA) __HAL_RCC_GPIOA_CLK_ENABLE();
    else if (port == GPIOB) __HAL_RCC_GPIOB_CLK_ENABLE();
    else if (port == GPIOC) __HAL_RCC_GPIOC_CLK_ENABLE();
    else if (port == GPIOD) __HAL_RCC_GPIOD_CLK_ENABLE();
    else if (port == GPIOE) __HAL_RCC_GPIOE_CLK_ENABLE();
    else if (port == GPIOF) __HAL_RCC_GPIOF_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(port, &GPIO_InitStruct);
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
```

---

## LANGKAH 3: Buat pins.h (FILE BARU) - ALL GPIO PIN DEFINITIONS

**Lokasi**: `Src/hwconf/pins.h`

**Isi file**:
```c
#ifndef PINS_H
#define PINS_H

// ===== MOTOR 1 GATE DRIVERS (TIM1) =====
#define GATE_M1_UH_PORT   GPIOC
#define GATE_M1_UH_PIN    GPIO_PIN_7

#define GATE_M1_UL_PORT   GPIOC
#define GATE_M1_UL_PIN    GPIO_PIN_8

#define GATE_M1_VH_PORT   GPIOC
#define GATE_M1_VH_PIN    GPIO_PIN_9

#define GATE_M1_VL_PORT   GPIOC
#define GATE_M1_VL_PIN    GPIO_PIN_10

#define GATE_M1_WH_PORT   GPIOC
#define GATE_M1_WH_PIN    GPIO_PIN_11

#define GATE_M1_WL_PORT   GPIOC
#define GATE_M1_WL_PIN    GPIO_PIN_12

// ===== MOTOR 2 GATE DRIVERS (TIM8) =====
#define GATE_M2_UH_PORT   GPIOE
#define GATE_M2_UH_PIN    GPIO_PIN_9

#define GATE_M2_UL_PORT   GPIOE
#define GATE_M2_UL_PIN    GPIO_PIN_8

#define GATE_M2_VH_PORT   GPIOE
#define GATE_M2_VH_PIN    GPIO_PIN_11

#define GATE_M2_VL_PORT   GPIOE
#define GATE_M2_VL_PIN    GPIO_PIN_10

#define GATE_M2_WH_PORT   GPIOE
#define GATE_M2_WH_PIN    GPIO_PIN_13

#define GATE_M2_WL_PORT   GPIOE
#define GATE_M2_WL_PIN    GPIO_PIN_12

// ===== FAULT INPUTS =====
#define FAULT_M1_PORT     GPIOE
#define FAULT_M1_PIN      GPIO_PIN_4

#define FAULT_M2_PORT     GPIOE
#define FAULT_M2_PIN      GPIO_PIN_3

// ===== LEDs =====
#define LED_PORT          GPIOB
#define LED_PIN           GPIO_PIN_12

// ===== HALL SENSORS MOTOR 1 =====
#define HALL_M1_A_PORT    GPIOE
#define HALL_M1_A_PIN     GPIO_PIN_8

#define HALL_M1_B_PORT    GPIOE
#define HALL_M1_B_PIN     GPIO_PIN_7

#define HALL_M1_C_PORT    GPIOE
#define HALL_M1_C_PIN     GPIO_PIN_6

// ===== HALL SENSORS MOTOR 2 =====
#define HALL_M2_A_PORT    GPIOD
#define HALL_M2_A_PIN     GPIO_PIN_8

#define HALL_M2_B_PORT    GPIOD
#define HALL_M2_B_PIN     GPIO_PIN_9

#define HALL_M2_C_PORT    GPIOD
#define HALL_M2_C_PIN     GPIO_PIN_10

// ===== I2C BIT-BANG PINS =====
#define I2C_SCL_PORT      GPIOB
#define I2C_SCL_PIN       GPIO_PIN_10

#define I2C_SDA_PORT      GPIOB
#define I2C_SDA_PIN       GPIO_PIN_11

#endif
```

---

## LANGKAH 4: UPDATE Src/hwconf/board.c

**CARI CODE INI**:
```c
void hw_init_gpio(void) {
    palSetPadMode(GPIOC, 7, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(GPIOC, 8, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(GPIOC, 9, PAL_MODE_OUTPUT_PUSHPULL);
    // ... dll semua palSetPadMode
}
```

**GANTI DENGAN**:
```c
#include "hwconf/hal_gpio.h"
#include "hwconf/pins.h"

void hw_init_gpio(void) {
    // Enable ALL GPIO clocks
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    
    // ===== MOTOR 1 GATE DRIVERS =====
    hal_gpio_init_output(GATE_M1_UH_PORT, GATE_M1_UH_PIN);
    hal_gpio_init_output(GATE_M1_UL_PORT, GATE_M1_UL_PIN);
    hal_gpio_init_output(GATE_M1_VH_PORT, GATE_M1_VH_PIN);
    hal_gpio_init_output(GATE_M1_VL_PORT, GATE_M1_VL_PIN);
    hal_gpio_init_output(GATE_M1_WH_PORT, GATE_M1_WH_PIN);
    hal_gpio_init_output(GATE_M1_WL_PORT, GATE_M1_WL_PIN);
    
    // ===== MOTOR 2 GATE DRIVERS =====
    hal_gpio_init_output(GATE_M2_UH_PORT, GATE_M2_UH_PIN);
    hal_gpio_init_output(GATE_M2_UL_PORT, GATE_M2_UL_PIN);
    hal_gpio_init_output(GATE_M2_VH_PORT, GATE_M2_VH_PIN);
    hal_gpio_init_output(GATE_M2_VL_PORT, GATE_M2_VL_PIN);
    hal_gpio_init_output(GATE_M2_WH_PORT, GATE_M2_WH_PIN);
    hal_gpio_init_output(GATE_M2_WL_PORT, GATE_M2_WL_PIN);
    
    // ===== FAULT INPUTS =====
    hal_gpio_init_input(FAULT_M1_PORT, FAULT_M1_PIN);
    hal_gpio_init_input(FAULT_M2_PORT, FAULT_M2_PIN);
    
    // ===== LEDs =====
    hal_gpio_init_output(LED_PORT, LED_PIN);
    
    // ===== HALL SENSORS =====
    hal_gpio_init_input_pullup(HALL_M1_A_PORT, HALL_M1_A_PIN);
    hal_gpio_init_input_pullup(HALL_M1_B_PORT, HALL_M1_B_PIN);
    hal_gpio_init_input_pullup(HALL_M1_C_PORT, HALL_M1_C_PIN);
    
    hal_gpio_init_input_pullup(HALL_M2_A_PORT, HALL_M2_A_PIN);
    hal_gpio_init_input_pullup(HALL_M2_B_PORT, HALL_M2_B_PIN);
    hal_gpio_init_input_pullup(HALL_M2_C_PORT, HALL_M2_C_PIN);
    
    // ===== I2C BIT-BANG PINS =====
    hal_gpio_init_input_pullup(I2C_SCL_PORT, I2C_SCL_PIN);
    hal_gpio_init_input_pullup(I2C_SDA_PORT, I2C_SDA_PIN);
    
    // Set initial gate states to LOW (safe)
    hal_gpio_write(GATE_M1_UH_PORT, GATE_M1_UH_PIN, 0);
    hal_gpio_write(GATE_M1_UL_PORT, GATE_M1_UL_PIN, 0);
    hal_gpio_write(GATE_M1_VH_PORT, GATE_M1_VH_PIN, 0);
    hal_gpio_write(GATE_M1_VL_PORT, GATE_M1_VL_PIN, 0);
    hal_gpio_write(GATE_M1_WH_PORT, GATE_M1_WH_PIN, 0);
    hal_gpio_write(GATE_M1_WL_PORT, GATE_M1_WL_PIN, 0);
    
    hal_gpio_write(GATE_M2_UH_PORT, GATE_M2_UH_PIN, 0);
    hal_gpio_write(GATE_M2_UL_PORT, GATE_M2_UL_PIN, 0);
    hal_gpio_write(GATE_M2_VH_PORT, GATE_M2_VH_PIN, 0);
    hal_gpio_write(GATE_M2_VL_PORT, GATE_M2_VL_PIN, 0);
    hal_gpio_write(GATE_M2_WH_PORT, GATE_M2_WH_PIN, 0);
    hal_gpio_write(GATE_M2_WL_PORT, GATE_M2_WL_PIN, 0);
}
```

---

## LANGKAH 5: FIND & REPLACE palReadPad - FILE DEMI FILE

### Src/applications/app_adc.c

**CARI**:
```c
bool button_state = palReadPad(GPIOE, 15);
```

**GANTI DENGAN**:
```c
#include "hwconf/hal_gpio.h"
bool button_state = hal_gpio_read(GPIOE, GPIO_PIN_15);
```

### Src/applications/app_nunchuk.c

**CARI**:
```c
palReadPad(GPIOB, 5)
palWritePad(GPIOB, 5, state)
```

**GANTI DENGAN**:
```c
#include "hwconf/hal_gpio.h"
hal_gpio_read(GPIOB, GPIO_PIN_5)
hal_gpio_write(GPIOB, GPIO_PIN_5, state)
```

### Src/applications/app_ppm.c

**CARI**:
```c
palReadPad(...) untuk semua input
```

**GANTI DENGAN**:
```c
#include "hwconf/hal_gpio.h"
hal_gpio_read(port, pin)
```

### Src/encoder/* (semua file encoder)

**CARI**:
```c
palReadPad untuk hall sensor reads
palWritePad untuk output chip select
palSetPadMode untuk pin configuration
```

**GANTI DENGAN**:
```c
#include "hwconf/hal_gpio.h"
hal_gpio_read(HALL_M1_A_PORT, HALL_M1_A_PIN)
hal_gpio_write(CS_PORT, CS_PIN, state)
hal_gpio_init_output(port, pin)
```

### Src/driver/i2c_bb.c

**CARI**:
```c
palSetPadMode(GPIOB, 10, PAL_MODE_OUTPUT_PUSHPULL);
palSetPadMode(GPIOB, 11, PAL_MODE_OUTPUT_PUSHPULL);
palWritePad(GPIOB, 10, 1);  // SCL high
palReadPad(GPIOB, 11);      // SDA read
```

**GANTI DENGAN**:
```c
#include "hwconf/hal_gpio.h"
#include "hwconf/pins.h"

// SCL and SDA configuration
hal_gpio_init_input_pullup(I2C_SCL_PORT, I2C_SCL_PIN);
hal_gpio_init_input_pullup(I2C_SDA_PORT, I2C_SDA_PIN);

// SCL high (open-drain, set to input = pulled high by resistor)
hal_gpio_init_input(I2C_SCL_PORT, I2C_SCL_PIN);
// SDA read
bool sda_state = hal_gpio_read(I2C_SDA_PORT, I2C_SDA_PIN);
```

### Src/driver/spi_bb.c (Bit-bang SPI jika ada)

**CARI**:
```c
palWritePad(GPIOC, 4, 1);  // CS high
palReadPad(GPIOC, 5);      // MISO read
```

**GANTI DENGAN**:
```c
#include "hwconf/hal_gpio.h"
hal_gpio_write(GPIOC, GPIO_PIN_4, 1);   // CS high
bool miso = hal_gpio_read(GPIOC, GPIO_PIN_5);  // MISO read
```

### Src/imu/imu.c

**CARI**:
```c
palWritePad untuk chip select
palReadPad untuk interrupt pin
```

**GANTI DENGAN**:
```c
#include "hwconf/hal_gpio.h"
hal_gpio_write(CS_PORT, CS_PIN, state);
bool irq = hal_gpio_read(IRQ_PORT, IRQ_PIN);
```

---

## LANGKAH 6: MOTOR ISR GATE DRIVE - NO FUNCTION CALL

**PENTING**: Di file Src/motor/mcpwm_foc.c, untuk gate drivers di dalam ISR, gunakan macro langsung (NO hal_gpio_write call untuk latency)

**CARI**:
```c
void ADC_handler(...) {
    // ... update PWM
    TIM1->CCR1 = duty; // existing code
}
```

**Untuk gate outputs (jika ada), REPLACE DENGAN**:
```c
#include "hwconf/pins.h"

void ADC_handler(...) {
    // Direct register access (NO FUNCTION CALL)
    if (gate_u_high) {
        GPIO_PIN_SET_DIRECT(GATE_M1_UH_PORT, GATE_M1_UH_PIN);
    } else {
        GPIO_PIN_RESET_DIRECT(GATE_M1_UH_PORT, GATE_M1_UH_PIN);
    }
    // ... repeat for all 6 phases
}
```

---

## CHECKLIST AGENT 1 COMPLETE

- [ ] `Src/hwconf/hal_gpio.h` created
- [ ] `Src/hwconf/hal_gpio.c` created  
- [ ] `Src/hwconf/pins.h` created
- [ ] `Src/hwconf/board.c` updated - semua `palSetPadMode` → `hal_gpio_init_*`
- [ ] `Src/applications/app_*.c` - semua `palReadPad/palWritePad` → `hal_gpio_read/write`
- [ ] `Src/encoder/*.c` - semua GPIO calls replaced
- [ ] `Src/driver/i2c_bb.c` - semua I2C GPIO replaced
- [ ] `Src/driver/spi_bb.c` - semua SPI GPIO replaced (jika ada)
- [ ] `Src/motor/mcpwm_foc.c` - motor ISR menggunakan macro langsung
- [ ] Zero `pal*` references remain (cek dengan grep)
- [ ] Compile tanpa error

---
