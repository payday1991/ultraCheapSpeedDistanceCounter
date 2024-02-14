#include "glcd.h"

#include "visuals.h" 
#include "stdio.h"
#include "stdlib.h"

#include "fonts/Calibri23x38.h"
#include "include/machineData.h"
#include "fonts/battery8x8.h"
#include "fonts/font5x7.h"
#include "fonts/font13x14.h"

//Our magic wand to save about !!!2KB!!! of flash memory.
extern int mini_snprintf(char* buffer, unsigned int buffer_len, const char *fmt, ...);




void showLogo(const unsigned char *data) {
	/*Show CBS logo*/
	glcd_draw_bitmap(data); //something instead...
	glcd_write();
	Delay_Ms(3000);
	/*Go to the next screen*/
	machineData.visuals.currentScreen++;
}



/**
 * @brief Display a small battery symbol in the top-right corner. Blinks when charging.
 * 
 */
static void showBatteryState (void) {
	glcd_tiny_set_font(battery8x8,8,8,0,6);
	if (machineData.flags.batteryCharging == true) {
		static _Bool batteryChargingBlinkIcon;
		static uint8_t batteryChargingBlinkIconPeriodDivider = BATTERY_CHARGING_BLINK_PERIOD_DIVIDER;

		batteryChargingBlinkIconPeriodDivider--;
		if (batteryChargingBlinkIconPeriodDivider == 0) {
			batteryChargingBlinkIcon = !batteryChargingBlinkIcon;
			batteryChargingBlinkIconPeriodDivider = BATTERY_CHARGING_BLINK_PERIOD_DIVIDER;
		}
			if (batteryChargingBlinkIcon) {
				glcd_draw_char_xy(119, 0, machineData.machine.batteryState); //Show the current state of charge
			}
			else {
				glcd_draw_char_xy(119, 0, 6); //Show no battery icon
			}

	}
	else {
		glcd_draw_char_xy(119, 0, machineData.machine.batteryState); //Show the current state of charge
	}
}



/**
 * @brief Displays the temperature on the screen.
 * 
 * This function shows the temperature on the screen using the GLCD library.
 * It formats the temperature value as a string and then draws the string on the screen.
 * 
 * @note The temperature value is obtained from the `machineData` structure.
 * 
 * @note The temperature is displayed using the Font5x7 font with a size of 5x7 pixels.
 * 
 * @note The temperature string is limited to 8 characters, including the 'C' symbol.
 * 
 * @note The temperature string is drawn at the coordinates (95, 0) on the screen.
 */
static void showTemperature (void) {
	char str[16] = {0};
	//Show the temperature on the screen
	glcd_tiny_set_font(Font5x7,5,7,32,127);
	mini_snprintf(str, 8, "%dC", machineData.machine.outsideTemperature);
	glcd_draw_string_xy(95, 0, str);
}



/**
 * @brief Screen where speed readout font is the biggest one on the screen.
 * 
 * @param machineData Pointer to the main data chunk structure
 */
static inline void showMainScreenSpeed (machineData_t* machineData) 
{
	char str[64] = {0};
	//Clean the buffer
	glcd_clear_buffer();

	//Show the speed on the screen
	glcd_tiny_set_font(Font5x7,5,7,32,127);
	glcd_draw_string_xy_P(13, 0, "Speed:");

	glcd_set_font(Calibri23x38,23,38,46,57);
	mini_snprintf(str, 3, "%2d", (int8_t)machineData->machine.speed);
	glcd_draw_string_xy(17, 8, str);

	glcd_tiny_set_font(Font5x7,5,7,32,127);
	glcd_draw_string_xy_P(13, 50, "m/min");

	//Draw separation lines
	glcd_draw_line(65, 0, 65, 64, 1);
	glcd_draw_line(66, 0, 66, 64, 1);

	glcd_draw_line(65, 33, 128, 33, 1);
	glcd_draw_line(65, 34, 128, 34, 1);

	//Show the time on the screen
	glcd_tiny_set_font(Font5x7,5,7,32,127);
	glcd_draw_string_xy_P(83, 9, "Time:");

	glcd_set_font(Trebuchet_MS13x14,13,14,32,127);
	mini_snprintf(str, 5, "%3u", machineData->machine.time);
	glcd_draw_string_xy(76, 17, str); 
	
	//Show the distance on the screen
	glcd_tiny_set_font(Font5x7,5,7,32,127);
	glcd_draw_string_xy_P(73, 38, "Distance:");

	glcd_set_font(Trebuchet_MS13x14,13,14,32,127); 
	mini_snprintf(str, 6, "%ldm", (machineData->machine.currentDistance)/10);
	glcd_draw_string_xy(70, 48, str);
}


/**
 * @brief Screen where distance readout font is the biggest one on the screen
 * 
 * @param machineData Pointer to the main data chunk structure
 */
static inline void showMainScreenDistance (machineData_t* machineData)
{
	char str[64] = {0};

	//Clean the buffer
	glcd_clear_buffer();

	//Show the distance on the screen
	glcd_tiny_set_font(Font5x7,5,7,32,127);
	glcd_draw_string_xy_P(5, 0, "Distance m:");

	glcd_set_font(Calibri23x38,23,38,46,57);
	mini_snprintf(str, 7, "%04ld.%ld", (machineData->machine.currentDistance/10), (machineData->machine.currentDistance%10));
	glcd_draw_string_xy(0, 9, str);

	//Show the time on the screen 
	glcd_tiny_set_font(Font5x7,5,7,32,127);
	glcd_draw_string_xy_P(75, 51, "Time:");

	glcd_set_font(Trebuchet_MS13x14,13,14,32,127);
	mini_snprintf(str, 5, "%3u", machineData->machine.time);
	glcd_draw_string_xy(102, 48, str);

	glcd_set_font(Trebuchet_MS13x14,13,14,32,127);
	mini_snprintf(str, 8, "%dm/min", machineData->machine.speed);
	glcd_draw_string_xy(3, 48, str);
}


/**
 * @brief Screen with all the mileage data.
 * 
 * @param machineData Pointer to the main data chunk structure
 */
static inline void showSettingsScreen (machineData_t* machineData) {
		char str[64] = {0};

		//Clean the buffer
		glcd_clear_buffer();

		//Show the distance on the screen
		glcd_tiny_set_font(Font5x7,5,7,32,127);
		glcd_draw_string_xy_P(0, 0, "Machine mileage:");
		mini_snprintf(str, 14, "%lu m", mileageData.machineMileage/10);
		glcd_draw_string_xy(0, 10, str);

		glcd_draw_string_xy_P(0, 20, "Service in:");
		if (mileageData.serviceOverdue <= 0) {
			mini_snprintf(str, 24, "Overdue by %ld m", mileageData.serviceOverdue/10);
			glcd_draw_string_xy_P(0, 30, "");
		}
		else {
			mini_snprintf(str, 20, "%ld m", mileageData.serviceOverdue/10);
			glcd_draw_string_xy(0, 30, str);
		}
		glcd_draw_string_xy(0, 30, str);

		//Show the machine mileage
		glcd_draw_string_xy_P(0, 41, "On time age:");
		mini_snprintf(str, 14, "%u min", mileageData.machineOnTimeAge);
		glcd_draw_string_xy(0, 50, str);
}


#if defined(USE_TEMPERATURE_HUMIDITY_SENSOR)
/**
 * @brief Displays the temperature and humidity screen on the LCD.
 * 
 * This function clears the LCD buffer and then displays the temperature and humidity values
 * on the screen. It uses the machineData structure to retrieve the temperature and humidity
 * values and formats them into strings before displaying them on the LCD.
 * 
 * @param machineData Pointer to the machineData_t structure containing the temperature and humidity values.
 */ 
static inline void showTemperatureHumidityScreen (machineData_t* machineData) {
	char str[64] = {0};

	//Clean the buffer
	glcd_clear_buffer(); 
	
	glcd_tiny_set_font(Font5x7,5,7,32,127);
	glcd_draw_string_xy_P(0, 15, "Temperature");
	glcd_draw_string_xy_P(80, 15, "Humidity"); 

	glcd_draw_rect(3, 29, 50, 25, 1);
	glcd_draw_rect(80, 29, 40, 25, 1);

	glcd_set_font(Trebuchet_MS13x14,13,14,32,127);
	//Show the temperature on the screen
	mini_snprintf(str, 8, "%d*C", machineData->machine.outsideTemperature); 
	glcd_draw_string_xy(10, 35, str);

	//Show the humidity on the screen 
	mini_snprintf(str, 8, "%u%%", machineData->machine.outsideHumidity);
	glcd_draw_string_xy(85, 35, str);
}
#endif


/**
 * @brief Draw "Battery low!" message in the middle of the screen
 * 
 */
static inline void showLowBatteryScreen (void) {
	glcd_tiny_set_font(Font5x7,5,7,32,127);
	glcd_draw_string_xy_P(0, 30, "Battery low!");
	glcd_write();
}



/**
 * @brief Draw "Service me!" message in the middle of the screen
 * 
 */
static inline void showServiceMeScreen(void) {
	//Show the message that machine needs to be serviced
	glcd_tiny_set_font(Font5x7,5,7,32,127);
	glcd_draw_string_xy_P(0, 30, "Service me!");
	glcd_write();
	Delay_Ms(3000);
	glcd_clear();
	/*Go to the next screen*/
	machineData.visuals.currentScreen = mainScreenDistance;
} 



void updateScreen (machineData_t* machineData) { 
	switch (machineData->visuals.currentScreen) {
				case logoScreen:
					showLogo(cbs);
					break;

				case mainScreenSpeed:
					showMainScreenSpeed(machineData);
					#if defined(USE_TEMPERATURE_HUMIDITY_SENSOR)
					showTemperature();
					#endif
					break;

				case mainScreenDistance:
					showMainScreenDistance(machineData);
					#if defined(USE_TEMPERATURE_HUMIDITY_SENSOR)
					showTemperature();
					#endif
					break;

				case temperatureHumidityScreen:
					#if defined(USE_TEMPERATURE_HUMIDITY_SENSOR)
					showTemperatureHumidityScreen(machineData);
					#else
					machineData->visuals.currentScreen++;
					#endif
					break;

				case settingsScreen:
					showSettingsScreen(machineData);
					break;

				case lowBatteryScreen:
					showLowBatteryScreen();
					break;

				case serviceMeScreen:
					showServiceMeScreen();
					break;

				default:
					break;
			}
			//Draw battery symbol according to the current battery state 
			showBatteryState(); 
			glcd_write();
}
