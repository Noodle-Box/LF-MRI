/*
 * DDS_Driver.h
 *
 *  Created on: Feb 3, 2025
 *      Author: Tevyn
 */

#ifndef SRC_DDS_DRIVER_H_
#define SRC_DDS_DRIVER_H_

#include "stm32g4xx_hal.h"

// SPI handle (must be initialized in main.c)
extern SPI_HandleTypeDef hspi2;

// AD9833 Master Clock Frequency
#define AD9833_MCLK (25000000ULL) // 25 MHz external clock

// AD9833 Control Macros
#define AD9833_B28           (1 << 13)  // Enable 28-bit frequency writes
#define AD9833_HLB           (1 << 12)  // High/Low byte access mode
#define AD9833_FSELECT       (1 << 11)  // Select FREQ1 register
#define AD9833_PSELECT       (1 << 10)  // Select PHASE1 register
#define AD9833_RESET         (1 << 8)   // Reset AD9833 (Set DAC to midscale)
#define AD9833_SLEEP1        (1 << 7)   // Power down internal clock
#define AD9833_SLEEP12       (1 << 6)   // Power down DAC
#define AD9833_OPBITEN       (1 << 5)   // Output MSB directly (for square wave)
#define AD9833_DIV2          (1 << 3)   // Divide output by 2
#define AD9833_MODE          (1 << 1)   // Triangle wave enable

// Function Prototypes
void AD9833_Init(void);
void AD9833_SetFrequency(uint32_t freq);
void AD9833_SetPhase(uint16_t phase);
void AD9833_SendCommand(uint16_t cmd);
void AD9833_EnableContinuousOutput(void);

#endif /* SRC_DDS_DRIVER_H_ */
