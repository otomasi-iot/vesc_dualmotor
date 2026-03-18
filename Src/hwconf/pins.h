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
