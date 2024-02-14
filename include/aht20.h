#pragma once

#include "ch32v003fun/ch32v003fun.h"
#include <stdbool.h>
#include <stdint.h>

// AHT20 I2C address
#define AHT20_I2C_ADDR 0x38

// I2C Timeout count
#define TIMEOUT_MAX 100000

// event codes we use
#define  I2C_EVENT_MASTER_MODE_SELECT 					((uint32_t)0x00030001)  /* BUSY, MSL and SB flag */
#define  I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED 	((uint32_t)0x00070082)  /* BUSY, MSL, ADDR, TXE and TRA flags */
#define  I2C_EVENT_MASTER_BYTE_TRANSMITTED 				((uint32_t)0x00070084)  /* TRA, BUSY, MSL, TXE and BTF flags */
#define  I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED        ((uint32_t)0x00030002)  /* BUSY, MSL and ADDR flags */

#define READ 1
#define WRITE 0

#define ERROR 1
#define OK 0



/**
 * @brief Reads temperature and humidity data from AHT20 sensor.
 *
 * This function reads temperature and humidity data from the AHT20 sensor using I2C.
 * It sends the necessary commands to the sensor, waits for the data to be ready, and then reads the data.
 * The temperature and humidity values are calculated from the raw data and stored in the provided variables.
 *
 * @param[out] tem Pointer to the variable where the temperature value will be stored.
 * @param[out] hum Pointer to the variable where the humidity value will be stored.
 */ 
bool aht20read(int8_t *tem, uint8_t *hum);



/**
 * @brief Initializes the AHT20 sensor.
 * 
 * This function initializes the AHT20 sensor by performing the following steps:
 * 1. Reads a single byte from the sensor using I2C communication.
 * 2. Checks if the 4th bit of the received byte is set to 0.
 * 3. If the 4th bit is not set to 0, sends a 3-byte buffer to the sensor using I2C communication.
 * 
 * @return Returns OK if the initialization is successful, otherwise returns ERROR.
 */
bool aht20init(void);