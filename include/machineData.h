/*
 * machineData.h
 *
 *  Created on: May 11, 2023
 *      Author: buncl
 */

#ifndef INC_MACHINEDATA_H_
#define INC_MACHINEDATA_H_

#include "stdint.h"

/*Battery state of charge*/
typedef enum  {
    flat,
    tenPercent,
    thirtyPercent,
    fiftyPercent,
    eightyPercent,
    full,
} batteryState_e;

/*Screens we got*/
typedef enum {logoScreen, mainScreenDistance, mainScreenSpeed, temperatureHumidityScreen, settingsScreen, serviceMeScreen, lowBatteryScreen} currentScreen_e;

/*The main chunk of data*/
typedef struct {
	struct machine {
		uint16_t time; //In minutes
		uint32_t currentDistance; //In 0.1m
		int16_t speed; //In m/min
		uint16_t batteryVoltage; //In mV
		int8_t batteryTemperature; //In C
		uint8_t outsideHumidity; //In %
		int8_t outsideTemperature; //In C 
		batteryState_e batteryState; //0 is flat, 1 is 10%, 2 is 30%, 3 is 50%, 4 is 80%, 5 is full
	}machine;

	struct visuals{
		uint8_t backlight; //Backlight brightness
		currentScreen_e currentScreen;
	}visuals;

	struct flags{
		uint8_t batteryNeedsMeasuring:1;
		uint8_t screenNeedsUpdating:1;
		uint8_t batteryCharging:1;
		uint8_t batteryFullyCharged:1;
		uint8_t needsServicing:1;
		uint8_t backlightOnRq:1;
		uint8_t backlightOffRq:1;
		uint8_t temperatureAndHumidityNeedsMeasuring:1;
		//uint8_t :0;
	}flags;
}machineData_t;
extern machineData_t machineData; // volatile

/*To store the machine mileage. It should be defined in .noinit region to prevent it from losing after reset*/
typedef struct {
	uint32_t machineMileage; //In 0.1m 
	uint32_t currentDistance; //In 0.1m
	uint16_t currentTime; //In minutes
	uint16_t machineOnTimeAge; //In minutes
	int32_t serviceOverdue; //In m. When goes to 0 and below, it means that service is overdue.
}mileageData_t;
extern mileageData_t mileageData;

#endif /* INC_MACHINEDATA_H_ */
