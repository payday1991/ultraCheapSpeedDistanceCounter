#pragma once

#include "include/main.h" 
#include "include/machineData.h"

//External variables
extern uint32_t sysTickCnt;

/*Bitmap pictures*/
extern const unsigned char logo[];

/**
 * @brief Show the logo for 3 seconds after powering up.
 * 
 * @param data 
 */
extern void showLogo (const unsigned char *data); 

/**
 * @brief Update/load(if machineData.visuals.currentScreen doesn't match to what we are showing now.) data on the screen.
 * 
 * @param machineData Pointer to the main data chunk structure to get
 */
extern void updateScreen (machineData_t* machineData);
