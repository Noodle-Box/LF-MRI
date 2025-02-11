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
    HAL_GPIO_WritePin(AD9833_FSYNC_PORT, AD9833_FSYNC_PIN, GPIO_PIN_RESET);
    HAL_SPI_Transmit(AD9833_SPI, (uint8_t*)&cmd, 1, HAL_MAX_DELAY);
    HAL_Delay(1);
    HAL_GPIO_WritePin(AD9833_FSYNC_PORT, AD9833_FSYNC_PIN, GPIO_PIN_SET);
}

/* Initialise AD9833 */
void AD9833_Init(void) {
    // Step 1: Reset AD9833 and enable 28-bit mode
    AD9833_SendCommand(0x2100);		// Control Register
    //AD9833_SendCommand(0x50C7); 	// Frequency Register 0 LSB
    //AD9833_SendCommand(0x4000); 	// Frequency Register 0 MSB

    // Step 2: Exit reset and enable waveform generation
    AD9833_SendCommand(0x2000); 	// Select FREQ0 and PHASE0
}

void AD9833_SetFrequency(uint32_t freq) {
    //uint64_t freq_reg = ((uint64_t)freq * 268435456ULL) / AD9833_MCLK;

	// uint32_t freq_reg = (uint32_t)(((uint64_t)freq * 10737UL + 500UL) / 1000UL); // 2^28 * f / f_MCLK
	// uint32_t freq;
	uint32_t freq_reg = (uint32_t)(((uint64_t)freq * 268435456ULL) / AD9833_MCLK);

	freq_reg &= 0x0FFFFFFFUL; 									// Ensure frequency data fits in 28-bit range
	uint16_t freq_LSB = (freq_reg & 0x3FFF) | 0x4000; 			// Lower 14 bits with control
	uint16_t freq_MSB = ((freq_reg >> 14) & 0x3FFF) | 0x8000; 	// Upper 14 bits with control

    char buffer[128];  // Increased buffer size for clarity

    // Debug Output to USB CDC Terminal
    sprintf(buffer, "Requested Frequency: %lu Hz\n", freq);
    CDC_Transmit_FS((uint8_t*)buffer, strlen(buffer));
    HAL_Delay(10);


    /* DEBUG THIS CAUSE IT AIN'T WORKING
     *
     * Calc 1000Hz = 0x29F1
     * Calc 5000Hz = 0xD1B7
     *
     * Calc 100kHz = 0x10624D
     * Calc 500kHz = 0x51EB85
     */
    sprintf(buffer, "Calculated FREQREG: 0x%08X\n", freq_reg);
    CDC_Transmit_FS((uint8_t*)buffer, strlen(buffer));
    HAL_Delay(10);

    sprintf(buffer, "FREQ_LSB: 0x%04X | FREQ_MSB: 0x%04X\n", freq_LSB, freq_MSB);
    CDC_Transmit_FS((uint8_t*)buffer, strlen(buffer));
    HAL_Delay(10);

    // Enable writing to FREQ0 register (B28 mode)
    AD9833_SendCommand(AD9833_B28 | 0x2000);

    // Write LSB first, then MSB
    AD9833_SendCommand(0x4000 | freq_LSB);
    AD9833_SendCommand(0x8000 | freq_MSB);
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





