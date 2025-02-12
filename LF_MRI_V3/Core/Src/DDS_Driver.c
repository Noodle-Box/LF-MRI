/************************************************************************************
 *
 * DDS Library for AD9833
 * Project: Low Field - MRI
 * UQ Centre for Advanced Engineering
 *
 * Author: Tevyn Vergara
 *
 ***********************************************************************************/
#include <math.h>
#include <stdio.h>
#include "DDS_Driver.h"
#include "usb_device.h"
#include "stm32g4xx_hal.h"
#include "usbd_cdc_if.h"

#define AD9833_SPI &hspi2
#define AD9833_FSYNC_PIN GPIO_PIN_6
#define AD9833_FSYNC_PORT GPIOC

/* Helper function to send data */
void AD9833_SendCommand(uint16_t cmd) {
    uint8_t data[2];
    data[0] = (cmd >> 8) & 0xFF;
    data[1] = cmd & 0xFF;

    // Debug Print
    char buffer[128];
    sprintf(buffer, "SPI TX: 0x%04X | Bytes: 0x%02X 0x%02X\r\n", cmd, data[0], data[1]);
    CDC_Transmit_FS((uint8_t*)buffer, strlen(buffer));
    HAL_Delay(10);

    // **Ensure FSYNC is HIGH before sending**
    HAL_GPIO_WritePin(AD9833_FSYNC_PORT, AD9833_FSYNC_PIN, GPIO_PIN_SET);
    HAL_Delay(1); // Ensure timing stability

    // **Drop FSYNC LOW to start transmission**
    HAL_GPIO_WritePin(AD9833_FSYNC_PORT, AD9833_FSYNC_PIN, GPIO_PIN_RESET);
    HAL_SPI_Transmit(AD9833_SPI, data, 2, HAL_MAX_DELAY);

    // **Set FSYNC HIGH immediately after transmission**
    HAL_GPIO_WritePin(AD9833_FSYNC_PORT, AD9833_FSYNC_PIN, GPIO_PIN_SET);
}

/* Initialise AD9833 */
void AD9833_Init(void) {
    // Force AD9833 into reset mode
    AD9833_SendCommand(AD9833_RESET | AD9833_B28);
    HAL_Delay(10);

    // Set initial frequency (e.g., 1kHz)
    AD9833_SetFrequency(1000);

    // **Explicitly clear reset bit** to start waveform generation
    AD9833_SendCommand(0x2000);
}


void AD9833_SetFrequency(uint32_t freq) {
    uint32_t freq_reg = (uint32_t)(((uint64_t)freq * 268435456ULL) / AD9833_MCLK);
    freq_reg &= 0x0FFFFFFF;

    uint16_t freq_LSB = (freq_reg & 0x3FFF);
    uint16_t freq_MSB = ((freq_reg >> 14) & 0x3FFF);

    // Add a separator before printing new frequency

    char buffer[128];
    sprintf(buffer, "\r\n=================================\r\n");
    CDC_Transmit_FS((uint8_t*)buffer, strlen(buffer));
    HAL_Delay(10);

    sprintf(buffer, "Setting Frequency: %lu Hz\r\n", freq);
    CDC_Transmit_FS((uint8_t*)buffer, strlen(buffer));
    HAL_Delay(10);

    sprintf(buffer, "FREQ_LSB: 0x%04X | FREQ_MSB: 0x%04X\r\n", freq_LSB, freq_MSB);
    CDC_Transmit_FS((uint8_t*)buffer, strlen(buffer));
    HAL_Delay(10);

    // Send LSB first, then MSB
    AD9833_SendCommand(0x4000 | freq_LSB);
    AD9833_SendCommand(0x8000 | freq_MSB);

    // **Extra clock cycle to force register refresh**
    AD9833_SendCommand(0xC000);  // Phase register write (forces update)

    // Explicitly enable waveform generation again
    AD9833_SendCommand(0x2000);
}


/* Set phase */
void AD9833_SetPhase(uint16_t phase) {
    uint16_t phase_val = (phase * 4096) / 360;
    AD9833_SendCommand(0xC000 | (phase_val & 0x0FFF)); // Write to PHASE0
}

/* Enable continuous waveform output */
void AD9833_EnableContinuousOutput(void) {
    AD9833_SendCommand(0x2000); // Select FREQ0 and PHASE0
    AD9833_SendCommand(0x0000); // Ensure DAC is enabled and sine wave mode is active
}
