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

#define SPI 			&hspi2
#define PORT1 			GPIOB
#define PORT2 			GPIOC
#define DDS_CHIP_SELECT GPIO_PIN_6
#define DDS_SCLK 		GPIO_PIN_13
#define DDS_MOSI_DATA   GPIO_PIN_15

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
    HAL_GPIO_WritePin(PORT2, DDS_CHIP_SELECT, GPIO_PIN_SET);
    HAL_Delay(1); // Ensure timing stability

    // **Drop FSYNC LOW to start transmission**
    HAL_GPIO_WritePin(PORT2, DDS_CHIP_SELECT, GPIO_PIN_RESET);
    HAL_SPI_Transmit(SPI, data, 2, HAL_MAX_DELAY);

    // **Set FSYNC HIGH immediately after transmission**
    HAL_GPIO_WritePin(PORT2, DDS_CHIP_SELECT, GPIO_PIN_SET);
}


/* Init for AD9833 DDS */
void AD9833_Init(void) {
	HAL_GPIO_WritePin(PORT1, DDS_MOSI_DATA, GPIO_PIN_SET); // Set All SPI pings to High
	HAL_GPIO_WritePin(PORT1, DDS_SCLK, GPIO_PIN_SET);  // Set All SPI pings to High
	HAL_GPIO_WritePin(PORT2, DDS_CHIP_SELECT, GPIO_PIN_SET);   // Set All SPI pings to High
}

/*
 * Function to set wave frequency and phase data
 *
 * Variables:
 * freq = frequency data to be sent to DDS
 * phase = phase data to be sent to DDS
 */
void AD9833_WaveSet(uint32_t freq, uint16_t phase) {

	// Frequency math
    uint32_t freq_reg = (uint32_t)(((uint64_t)freq * 268435456ULL) / AD9833_MCLK);
    freq_reg &= 0x0FFFFFFF;

    uint16_t freq_LSB = (freq_reg & 0x3FFF);
    uint16_t freq_MSB = ((freq_reg >> 14) & 0x3FFF);

    // Phase math
    uint16_t phase_val = (phase * 4096) / 360;
    phase_val &= 0x0FFF;

    // Toggle control register
    AD9833_SendCommand(0x2100);

    // Frequency Register LSB
    AD9833_SendCommand(0x4000 | freq_LSB);

    // Frequency Register MSB
    AD9833_SendCommand(0x8000 | freq_MSB);

    // Phase Register
    AD9833_SendCommand(0xC000 | phase_val);

    // Exit reset
    AD9833_SendCommand(0x2000);

    // Terminla printing for debugging
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

    sprintf(buffer, "FREQ_LSB: 0x%04X | FREQ_MSB: 0x%04X\r\n", freq_LSB, freq_MSB);
    CDC_Transmit_FS((uint8_t*)buffer, strlen(buffer));
    HAL_Delay(10);
}

/* Enable continuous waveform output
void AD9833_EnableContinuousOutput(void) {
    AD9833_SendCommand(0x2000); // Select FREQ0 and PHASE0
    AD9833_SendCommand(0x0000); // Ensure DAC is enabled and sine wave mode is active

} */



/* DDS DEBUG NOTES
 *
 * 1kHz = 0x29F1
 * 1MHz =
 */
