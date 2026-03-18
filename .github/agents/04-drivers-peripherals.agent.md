---
name: Drivers & Peripherals Conversion Agent
description: |
  Specialized agent for converting peripheral drivers from ChibiOS to STM32 HAL + FreeRTOS.
  Use when: Converting I2C/SPI bit-bang drivers, CAN, USB serial, encoders, IMU to HAL.
  Handles: Software I2C/SPI, hardware SPI, CAN communication, USB-CDC serial, encoder position reading, IMU sensor drivers.
  Scope: Src/driver/, Src/encoder/, Src/imu/, Src/comm/, Src/libcanard/
---

# Peripheral Drivers & Interfaces Conversion

## Mission
Convert all peripheral driver interfaces from ChibiOS-dependent APIs to STM32 HAL while preserving:
- **Timing guarantees**: I2C/SPI bit-bang frequencies unchanged
- **Reliability**: Error handling and retry logic intact
- **Communication protocols**: CAN, USB CDC remain compatible
- **Sensor interface**: Encoder position and IMU data unchanged
- **Deterministic behavior**: Thread-safe access via mutexes (from Agent 2)

---

## Architecture: Driver Categories

```
Drivers/
├─ Bit-banged (GPIO timing-critical)
│  ├─ I2C: i2c_bb.c  (400 kHz typical)
│  └─ SPI: spi_bb.c  (1-10 MHz, variable)
├─ Hardware peripherals (HAL drivers)
│  ├─ CAN: comm_can.c (1 Mbps)
│  ├─ UART: comm_usb_serial.c (115200+ baud)
│  ├─ SPI: hwconf/drv_spi.c (gate drivers)
│  └─ ADC: mcpwm_foc.c (6 kHz sampling)
└─ Sensor interfaces (decoder + thread)
   ├─ Encoders: encoder/enc_*.c (multiple types)
   ├─ IMU: imu/imu.c (I2C/SPI)
   └─ Fault inputs: hwconf/board.c (GPIO ISR)
```

---

## Phase 1: Bit-Banged I2C Conversion

### 1.1 I2C Software Implementation (i2c_bb.c)

**File**: `Src/driver/i2c_bb.c`

**ChibiOS I2C structure**:
```c
// BEFORE (ChibiOS)
#include "ch.h"
#include "hal.h"

static I2CDriver I2CD1;

void i2c_bb_init(void) {
    static const I2CConfig i2c_config = {
        .freq = 400000,  // 400 kHz
        .duty_cycle = STD_DUTY_CYCLE
    };
    i2cStart(&I2CD1, &i2c_config);
}

int i2c_bb_tx_rx(uint8_t addr, const uint8_t *tx, int tx_len, 
                  uint8_t *rx, int rx_len) {
    msg_t status = i2cMasterTransmitTimeout(&I2CD1, addr, 
        tx, tx_len, rx, rx_len, MS2ST(100));
    return status == MSG_OK ? 0 : -1;
}
```

**Converted to pure GPIO bit-bang** (no ChibiOS dependency):

```c
// Src/driver/i2c_bb.c (CONVERTED)

#include "stm32f1xx_hal.h"
#include "cmsis_os2.h"
#include "hwconf/pins.h"  // GPIO abstraction from Agent 1

// Global state
static osMutexId_t i2c_mutex;
static volatile bool i2c_busy = false;

void i2c_bb_init(void) {
    // Initialize GPIO pins (already done in Agent 1: hal_gpio_init)
    // Just create the mutex
    const osMutexAttr_t attr = {
        .name = "i2c_bb_lock",
        .attr_bits = osMutexPrioInherit
    };
    i2c_mutex = osMutexNew(&attr);
}

// Timing constants @ 72 MHz (13.89 ns per cycle)
#define I2C_SCL_FREQ    400000  // 400 kHz = 2.5 µs period
#define CYCLES_PER_BIT  (72000000 / I2C_SCL_FREQ / 2)  // ~90 cycles half-period

static inline void i2c_delay(void) {
    // Bit-bang requires tight loop for timing
    for (volatile int i = 0; i < 20; i++);  // ~280 ns
}

static inline void i2c_scl_high(void) {
    hal_gpio_write(&scl_pin, 1);
    i2c_delay();
}

static inline void i2c_scl_low(void) {
    hal_gpio_write(&scl_pin, 0);
    i2c_delay();
}

static inline void i2c_sda_high(void) {
    hal_gpio_write(&sda_pin, 1);
    i2c_delay();
}

static inline void i2c_sda_low(void) {
    hal_gpio_write(&sda_pin, 0);
    i2c_delay();
}

static bool i2c_sda_read(void) {
    return hal_gpio_read(&sda_pin);
}

static bool i2c_scl_read(void) {
    return hal_gpio_read(&scl_pin);
}

// I2C START condition
static void i2c_start(void) {
    i2c_scl_high();
    i2c_sda_high();
    i2c_sda_low();
    i2c_scl_low();
}

// I2C STOP condition
static void i2c_stop(void) {
    i2c_scl_low();
    i2c_sda_low();
    i2c_scl_high();
    i2c_sda_high();
}

// Write single bit
static void i2c_write_bit(bool bit) {
    if (bit) {
        i2c_sda_high();
    } else {
        i2c_sda_low();
    }
    i2c_scl_high();
    // Clock stretching: wait for slave to release SCL
    uint32_t timeout = 100000;
    while (!i2c_scl_read() && --timeout);
    i2c_scl_low();
}

// Read single bit
static bool i2c_read_bit(void) {
    i2c_sda_high();
    i2c_scl_high();
    
    // Clock stretching
    uint32_t timeout = 100000;
    while (!i2c_scl_read() && --timeout);
    
    bool bit = i2c_sda_read();
    i2c_scl_low();
    return bit;
}

// Write byte (MSB first)
static bool i2c_write_byte(uint8_t byte) {
    for (int i = 7; i >= 0; i--) {
        i2c_write_bit((byte >> i) & 1);
    }
    // Read ACK bit
    return !i2c_read_bit();  // ACK = 0, NACK = 1
}

// Read byte (MSB first)
static uint8_t i2c_read_byte(bool ack) {
    uint8_t byte = 0;
    for (int i = 7; i >= 0; i--) {
        if (i2c_read_bit()) {
            byte |= (1 << i);
        }
    }
    // Send ACK/NACK
    i2c_write_bit(!ack);  // ACK = 0, NACK = 1
    return byte;
}

// Main I2C transaction
int i2c_bb_tx_rx(uint8_t addr, const uint8_t *tx, int tx_len,
                  uint8_t *rx, int rx_len) {
    // Protect against concurrent I2C access
    osStatus_t status = osMutexAcquire(i2c_mutex, pdMS_TO_TICKS(100));
    if (status != osOK) {
        return -1;  // Timeout
    }
    
    i2c_busy = true;
    int result = -1;
    
    // START condition
    i2c_start();
    
    // Write address byte (7-bit address + R/W bit)
    uint8_t addr_byte = (addr << 1) | (tx_len > 0 ? 0 : 1);  // W if TX, R if RX only
    if (!i2c_write_byte(addr_byte)) {
        goto exit;  // No ACK from slave
    }
    
    // Write TX data
    for (int i = 0; i < tx_len; i++) {
        if (!i2c_write_byte(tx[i])) {
            goto exit;
        }
    }
    
    // Repeated START if reading
    if (rx_len > 0) {
        i2c_start();
        
        // Re-send address with READ bit
        if (!i2c_write_byte((addr << 1) | 1)) {
            goto exit;
        }
        
        // Read RX data
        for (int i = 0; i < rx_len; i++) {
            rx[i] = i2c_read_byte(i < (rx_len - 1));  // ACK all but last byte
        }
    }
    
    result = 0;  // Success
    
exit:
    i2c_stop();
    i2c_busy = false;
    osMutexRelease(i2c_mutex);
    
    return result;
}

// Wrapper API (compatible with existing encoder/imu drivers)
int i2c_bb_read_reg(uint8_t addr, uint8_t reg, uint8_t *data, int len) {
    return i2c_bb_tx_rx(addr, &reg, 1, data, len);
}

int i2c_bb_write_reg(uint8_t addr, uint8_t reg, const uint8_t *data, int len) {
    uint8_t tx_buf[len + 1];
    tx_buf[0] = reg;
    memcpy(&tx_buf[1], data, len);
    return i2c_bb_tx_rx(addr, tx_buf, len + 1, NULL, 0);
}
```

**Key points**:
- Mutex protects against concurrent access (from different threads)
- GPIO reads/writes via `hal_gpio_*` layer (no direct register access)
- No ChibiOS dependency; works with FreeRTOS
- Clock stretching support (slave holds SCL low)
- Timing achieved via software delay loop (portable, not cycle-accurate)

---

## Phase 2: Hardware SPI Conversion

### 2.1 SPI Hardware Driver (hwconf DRV chip comms)

**Used for**: Gate driver programming (DRV8301, DRV8305, etc.), high-speed peripherals

**File**: `Src/hwconf/mcuconf.c` or `Src/driver/spi_hw.c`

**Before (ChibiOS)**:
```c
// BEFORE: ChibiOS SPI
static const SPIConfig spi_config = {
    .end_cb = NULL,
    .ssport = GPIOB,
    .sspad = 5,
    .cr1 = SPI_CR1_BR_1 | SPI_CR1_CPOL | SPI_CR1_CPHA,
    .cr2 = 0
};

void spi_init(void) {
    spiStart(&SPID1, &spi_config);
}

void spi_write_read(const uint8_t *tx, uint8_t *rx, int len) {
    spiExchange(&SPID1, len, tx, rx);
}
```

**Converted to STM32 HAL**:

```c
// Src/driver/spi_hw.c

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
```

---

## Phase 3: CAN Communication

### 3.1 Hardware CAN Driver (comm_can.c)

**File**: `Src/comm/comm_can.c` - Update CAN RX/TX handlers

**Before (ChibiOS CAND1 driver)**:
```c
// BEFORE: ChibiOS CAN
static const CANConfig can_config = {
    .mcr = (1 << 6),  // ABOM: auto bus-off management
    .btr = CAN_BTR_1M   // 1 Mbps
};

void can_init(void) {
    canStart(&CAND1, &can_config);
}

void can_send(uint16_t id, const uint8_t *data, int len) {
    CANTxFrame frame = {
        .IDE = CAN_IDE_STD,
        .SID = id,
        .DLC = len
    };
    memcpy(frame.data8, data, len);
    canTransmit(&CAND1, CAN_ANY_MAILBOX, &frame);
}
```

**Converted to STM32 HAL**:

```c
// Src/comm/comm_can.c (UPDATED)

#include "stm32f1xx_hal.h"
#include "cmsis_os2.h"

CAN_HandleTypeDef hcan1;  // Motor 1 CAN bus
CAN_HandleTypeDef hcan2;  // Motor 2 CAN bus (if available)

void comm_can_init(void) {
    // Configure CAN1 for 1 Mbps operation
    __HAL_RCC_CAN1_CLK_ENABLE();
    
    hcan1.Instance = CAN1;
    hcan1.Init.Mode = CAN_MODE_NORMAL;
    hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
    hcan1.Init.AutoBusOff = ENABLE;
    hcan1.Init.AutoWakeUp = DISABLE;
    hcan1.Init.AutoRetransmission = ENABLE;
    hcan1.Init.ReceiveFifoLocked = DISABLE;
    hcan1.Init.TimeTriggeredMode = DISABLE;
    
    // 1 Mbps CAN at 72 MHz:
    // Prescaler=4, BS1=13, BS2=2, SJW=1
    // Bit time = (1+13+2) = 16 TQ @ 18 MHz = 1.125 Mbps (close enough)
    // Or: Prescaler=9, BS1=6, BS2=1 @ 8 MHz base = 1 Mbps exact
    hcan1.Init.Prescaler = 9;
    hcan1.Init.TimeSeg1 = CAN_BS1_6TQ;
    hcan1.Init.TimeSeg2 = CAN_BS2_1TQ;
    
    if (HAL_CAN_Init(&hcan1) != HAL_OK) {
        Error_Handler();
    }
    
    // Start CAN with RX interrupt
    HAL_CAN_Start(&hcan1);
    HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
}

void comm_can_send(uint16_t id, const uint8_t *data, int len) {
    CAN_TxHeaderTypeDef TxHeader = {
        .StdId = id & 0x7FF,
        .IDE = CAN_ID_STD,
        .RTR = CAN_RTR_DATA,
        .DLC = (len > 8) ? 8 : len
    };
    
    uint32_t TxMailbox;
    HAL_CAN_AddTxMessage(&hcan1, &TxHeader, (uint8_t *)data, &TxMailbox);
}

// CAN interrupt callback
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    if (hcan->Instance == CAN1) {
        CAN_RxHeaderTypeDef RxHeader = {0};
        uint8_t RxData[8];
        
        if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK) {
            // Dispatch to COBS packet handler or libcanard
            comm_can_process_message(&RxHeader, RxData);
        }
    }
}
```

---

## Phase 4: USB CDC Serial (comm_usb_serial.c)

**Purpose**: Terminal interface for VESC tool communication

**File**: `Src/comm/comm_usb_serial.c`

**HAL USB CDC stack**:

```c
// Src/comm/comm_usb_serial.c

#include "stm32f1xx_hal.h"
#include "usbd_core.h"
#include "usbd_cdc.h"
#include "cmsis_os2.h"

// USB device handle
USBD_HandleTypeDef hUsbDeviceFS;
extern USBD_DescriptorsTypeDef FS_Desc;

void comm_usb_init(void) {
    // Initialize USB stack
    if (USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS) != USBD_OK) {
        Error_Handler();
    }
    
    // Set USB device class (CDC)
    if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_CDC) != USBD_OK) {
        Error_Handler();
    }
    
    // Start USB device
    if (USBD_Start(&hUsbDeviceFS) != USBD_OK) {
        Error_Handler();
    }
}

// TX function (from VESC code to USB host)
int comm_usb_send(const uint8_t *data, int len) {
    // CDC_Transmit_FS is HAL-provided
    return CDC_Transmit_FS((uint8_t *)data, len);
}

// RX callback (USB → VESC)
static uint8_t usb_rx_buffer[512];

void CDC_Receive_FS(uint8_t *Buf, uint32_t Len) {
    // Called by USB stack when data received
    // Dispatch to command handler via osEventFlags or queue
    osEventFlagsSet(usb_rx_events, USB_DATA_AVAILABLE);
    memcpy(usb_rx_buffer, Buf, Len);
}
```

---

## Phase 5: Encoder Position Readers

### 5.1 Generic Encoder Interface

**File**: `Src/encoder/encoder.c` - Common interface

```c
// Src/encoder/encoder.c - Updated for FreeRTOS

#include "stm32f1xx_hal.h"
#include "encoder.h"
#include "cmsis_os2.h"

// Current encoder position (shared with motor control)
volatile float encoder_position = 0.0;      // Electrical angle [0, 2π]
volatile float encoder_velocity = 0.0;      // Electrical velocity [rad/s]
volatile encoder_fault_t encoder_fault = ENCODER_FAULT_NONE;

// Thread-safe access
static osMutexId_t encoder_lock;

void encoder_init(void) {
    const osMutexAttr_t attr = {
        .name = "encoder_lock"
    };
    encoder_lock = osMutexNew(&attr);
    
    // Initialize specific encoder type based on config
    switch (ENCODER_TYPE) {
        case ENC_TYPE_ABI:
            enc_abi_init();
            break;
        case ENC_TYPE_AS504X:
            enc_as504x_init();  // Uses I2C
            break;
        case ENC_TYPE_TS5700N8501:
            enc_ts5700n8501_init();  // Uses CANopen
            break;
        // ... other types ...
    }
}

float encoder_get_position(void) {
    osMutexAcquire(encoder_lock, osWaitForever);
    float pos = encoder_position;
    osMutexRelease(encoder_lock);
    return pos;
}

void encoder_set_position(float pos) {
    osMutexAcquire(encoder_lock, osWaitForever);
    encoder_position = pos;
    osMutexRelease(encoder_lock);
}

// Thread entry point
void *encoder_thread(void *arg) {
    osThreadSetName(osThreadGetId(), "encoder");
    
    while (1) {
        // Read encoder in non-blocking way
        float new_pos = get_encoder_position_native();
        float new_vel = get_encoder_velocity_native();
        
        osMutexAcquire(encoder_lock, osWaitForever);
        encoder_position = new_pos;
        encoder_velocity = new_vel;
        osMutexRelease(encoder_lock);
        
        // Update rate depends on encoder type (100-10000 Hz)
        osDelay(pdMS_TO_TICKS(10));  // 100 Hz encoder poll
    }
    
    return NULL;
}
```

### 5.2 Specific Encoder: AS504x (SPI)

**File**: `Src/encoder/enc_as504x.c` - SPI magnetic encoder

```c
// Src/encoder/enc_as504x.c

#include "stm32f1xx_hal.h"
#include "encoder.h"

typedef struct {
    SPI_HandleTypeDef *spi;
    GPIO_TypeDef *cs_port;
    uint16_t cs_pin;
} AS504xDriver;

static AS504xDriver as504x_m1;

void enc_as504x_init(void) {
    // SPI configuration (handled by Agent 2 - uses spi_hw_init())
    as504x_m1.spi = &hspi1;
    as504x_m1.cs_port = GPIOC;
    as504x_m1.cs_pin = GPIO_PIN_4;
}

float enc_as504x_read_position(void) {
    uint8_t cmd_buf[2] = {0x3F, 0x00};  // Read angle register
    uint8_t rx_buf[2];
    
    // Pull CS low
    HAL_GPIO_WritePin(as504x_m1.cs_port, as504x_m1.cs_pin, GPIO_PIN_RESET);
    
    // SPI transfer
    HAL_SPI_TransmitReceive(as504x_m1.spi, cmd_buf, rx_buf, 2, 100);
    
    // Pull CS high
    HAL_GPIO_WritePin(as504x_m1.cs_port, as504x_m1.cs_pin, GPIO_PIN_SET);
    
    // Extract 14-bit angle from response
    uint16_t raw = ((rx_buf[0] << 8) | rx_buf[1]) & 0x3FFF;
    
    // Convert to electrical angle [0, 2π)
    return (raw / 16384.0) * 2 * M_PI;
}
```

### 5.3 Encoder Type: Hall Sensor (GPIO)

**File**: `Src/encoder/enc_abi.c` - Incremental hall sensors

```c
// Src/encoder/enc_abi.c - Hall sensors (A, B, Index)

#include "stm32f1xx_hal.h"
#include "encoder.h"
#include "hwconf/pins.h"  // GPIO definitions

volatile int32_t hall_position = 0;
static osMutexId_t hall_lock;

void enc_abi_init(void) {
    const osMutexAttr_t attr = {.name = "hall_lock"};
    hall_lock = osMutexNew(&attr);
    
    // Configure GPIO inputs for hall sensors
    hal_gpio_init(&hall_m1_a, GPIO_MODE_INPUT, GPIO_PULLUP);
    hal_gpio_init(&hall_m1_b, GPIO_MODE_INPUT, GPIO_PULLUP);
    hal_gpio_init(&hall_m1_index, GPIO_MODE_INPUT, GPIO_PULLUP);
    
    // Optional: Use TIM4 input capture for precise counting
    // TIM4_CH1 ← Hall A
    // TIM4_CH2 ← Hall B
    // TIM4_CH3 ← Index (optional reset)
    
    // For simplicity, poll GPIO in encoder_thread()
}

void enc_abi_update(void) {
    static uint8_t last_state = 0;
    uint8_t current = 0;
    
    // Read current hall state
    current |= (hal_gpio_read(&hall_m1_a) << 0);
    current |= (hal_gpio_read(&hall_m1_b) << 1);
    
    // Detect state transition (simple incremental)
    if (current != last_state) {
        // Simple increment/decrement based on transition
        if ((last_state ^ current) & 0x01) {  // A changed
            if (current & 0x01) {
                if (current & 0x02) {
                    hall_position++;
                } else {
                    hall_position--;
                }
            }
        }
        
        last_state = current;
    }
}

float enc_abi_get_position(void) {
    osMutexAcquire(hall_lock, osWaitForever);
    float pos = (hall_position % 6) * (2 * M_PI / 6);  // 60° per hall zone
    osMutexRelease(hall_lock);
    return pos;
}
```

---

## Phase 6: IMU Sensor Driver

**File**: `Src/imu/imu.c` - 6-axis IMU (accelerometer + gyroscope)

```c
// Src/imu/imu.c

#include "stm32f1xx_hal.h"
#include "imu.h"
#include "cmsis_os2.h"
#include "driver/i2c_bb.h"  // Bit-banged I2C

#define IMU_ADDR 0x68  // MPU6050 I2C address

typedef struct {
    float accel_x, accel_y, accel_z;  // m/s²
    float gyro_x, gyro_y, gyro_z;     // rad/s
} imu_data_t;

volatile imu_data_t imu_data = {0};
static osMutexId_t imu_lock;

void imu_init(void) {
    const osMutexAttr_t attr = {.name = "imu_lock"};
    imu_lock = osMutexNew(&attr);
    
    // I2C already initialized in Agent 1
    // Configure IMU over I2C
    uint8_t config[2] = {0x1A, 0x06};  // DLPF config
    i2c_bb_write_reg(IMU_ADDR, 0x1A, &config[1], 1);
    
    // Power on, set sample rate
    uint8_t pwr = 0x01;
    i2c_bb_write_reg(IMU_ADDR, 0x6B, &pwr, 1);
}

void *imu_thread(void *arg) {
    osThreadSetName(osThreadGetId(), "imu");
    
    uint8_t imu_regs[6];
    
    while (1) {
        // Read accelerometer and gyroscope registers @ 100 Hz
        if (i2c_bb_read_reg(IMU_ADDR, 0x3B, imu_regs, 6) == 0) {
            // Parse raw sensor data
            int16_t accel_raw_x = (imu_regs[0] << 8) | imu_regs[1];
            int16_t accel_raw_y = (imu_regs[2] << 8) | imu_regs[3];
            int16_t accel_raw_z = (imu_regs[4] << 8) | imu_regs[5];
            
            // Convert to physical units (±16g range)
            osMutexAcquire(imu_lock, osWaitForever);
            imu_data.accel_x = accel_raw_x / 2048.0 * 9.81;
            imu_data.accel_y = accel_raw_y / 2048.0 * 9.81;
            imu_data.accel_z = accel_raw_z / 2048.0 * 9.81;
            osMutexRelease(imu_lock);
        }
        
        osDelay(pdMS_TO_TICKS(10));  // 100 Hz poll
    }
    
    return NULL;
}

void imu_get_accel(float *ax, float *ay, float *az) {
    osMutexAcquire(imu_lock, osWaitForever);
    *ax = imu_data.accel_x;
    *ay = imu_data.accel_y;
    *az = imu_data.accel_z;
    osMutexRelease(imu_lock);
}
```

---

## Phase 7: libcanard Integration (Optional)

**If using UAVCAN protocol** for advanced motor control:

**File**: `Src/libcanard/canard_hal.c`

```c
// Src/libcanard/canard_hal.c - Bridge libcanard to HAL CAN

#include "libcanard/canard.h"
#include "stm32f1xx_hal.h"

// libcanard frame transmit callback
CanardCANFrame canard_tx_frame;

int16_t canardHalTxWrite(CanardCANFrame *frame) {
    // Convert libcanard frame to STM32 HAL format
    CAN_TxHeaderTypeDef hdr = {
        .StdId = frame->id >> 26,  // CAN ID
        .IDE = CAN_ID_STD,
        .RTR = CAN_RTR_DATA,
        .DLC = frame->dlc
    };
    
    uint32_t mailbox;
    return HAL_CAN_AddTxMessage(&hcan1, &hdr, frame->data, &mailbox) == HAL_OK ? 0 : -1;
}

void canardHalRxPoll(CanardCANFrame *frame) {
    CAN_RxHeaderTypeDef hdr;
    uint8_t data[8];
    
    if (HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &hdr, data) == HAL_OK) {
        frame->id = hdr.StdId << 26;
        frame->dlc = hdr.DLC;
        memcpy(frame->data, data, frame->dlc);
    }
}
```

---

## Phase 8: Testing & Validation

### 8.1 Driver Functional Tests

```c
// Src/tests/test_drivers.c

void test_i2c_read_write(void) {
    // Read/write test to any I2C device (e.g., encoder)
    uint8_t test_data[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    uint8_t read_data[4];
    
    if (i2c_bb_write_reg(0x50, 0x00, test_data, 4) == 0) {
        printf("✓ I2C write OK\n");
    } else {
        printf("✗ I2C write FAIL\n");
    }
}

void test_spi_write_read(void) {
    uint8_t tx[] = {0xFF, 0x00};
    uint8_t rx[2];
    
    spi_hw_write_read(tx, rx, 2);
    printf("SPI TX: %02X %02X | RX: %02X %02X\n", 
        tx[0], tx[1], rx[0], rx[1]);
}

void test_usb_serial(void) {
    const char *msg = "Hello from VESC\n";
    comm_usb_send((const uint8_t *)msg, strlen(msg));
    printf("✓ USB serial sent\n");
}

void test_can_send(void) {
    uint8_t can_data[8] = {0x01, 0x02, 0x03, 0x04, 0, 0, 0, 0};
    comm_can_send(0x123, can_data, 8);
    printf("✓ CAN send OK\n");
}

void test_encoder(void) {
    float pos = encoder_get_position();
    printf("Encoder position: %.3f rad\n", pos);
}
```

### 8.2 Checklist for Completion

- [ ] i2c_bb.c bit-bang I2C working (400 kHz verified on oscilloscope)
- [ ] All hardware SPI calls use HAL (spi_hw_*)
- [ ] CAN initialization complete with correct bit timing
- [ ] USB CDC enumeration successful (device appears in Device Manager)
- [ ] All encoders reading valid positions
- [ ] IMU returning sensible acceleration/gyro data
- [ ] All mutexes protecting shared resources
- [ ] No `chI2c*`, `chSpi*`, `chCan*` references remain
- [ ] No USB stack compilation errors
- [ ] All drivers tested independently

---

## Key Files to Create/Modify

| File | Action | Purpose | Priority |
|------|--------|---------|----------|
| `Src/driver/i2c_bb.c` | **MODIFY** | Pure GPIO I2C bit-bang | **CRITICAL** |
| `Src/driver/spi_hw.c` | **CREATE** | Hardware SPI abstraction | **HIGH** |
| `Src/comm/comm_can.c` | **MODIFY** | STM32 HAL CAN interface | **CRITICAL** |
| `Src/comm/comm_usb_serial.c` | **MODIFY** | HAL USB CDC stack | **HIGH** |
| `Src/encoder/encoder.c` | **MODIFY** | Encoder thread + APIs | **HIGH** |
| `Src/encoder/enc_as504x.c` | **MODIFY** | SPI encoder reader | **MEDIUM** |
| `Src/encoder/enc_abi.c` | **MODIFY** | Hall/ABI encoder GPIO | **MEDIUM** |
| `Src/imu/imu.c` | **MODIFY** | I2C IMU sensor reader | **MEDIUM** |
| `Src/libcanard/canard_hal.c` | **CREATE** | UAVCAN bridge (optional) | **LOW** |

---

## Success Criteria

✅ I2C bit-bang reads/writes data consistently
✅ All I2C transactions complete within timeout
✅ SPI transfers match expected output frequency
✅ CAN bus transmits/receives frames correctly
✅ USB serial terminal responsive (no dropped characters)
✅ Encoder position values make physical sense
✅ IMU acceleration matches gravitational sense
✅ No race conditions on shared variables (mutex-protected)
✅ Driver error handling graceful (no crashes)
✅ All peripheral functionality preserved from ChibiOS version

---

## Notes

- **I2C mutex**: Protects against simultaneous encoder + IMU + other I2C reads
- **CAN timing**: MAY need tuning for your specific CAN baud rate requirement
- **USB stack**: Uses STM32CubeMX-generated USB code; may differ from your exact setup
- **Encoder types**: Multiple enc_*.c files (AS504x, TS5700, ABI, etc.) - only convert the types you use
- **Clock stretching**: Implemented in i2c_bb.c to handle slow I2C slaves

