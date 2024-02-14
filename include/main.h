#pragma once

#include "ch32v003fun/ch32v003fun.h"
#include "extralibs/ch32v003_GPIO_branchless.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

/*--------------------------------------------------------------Battery stuff--------------------------------------------------------------*/
#define BATTERY_CHARGING_BLINK_PERIOD_DIVIDER 5u
#define BATTERY_CHARGING_OFFSET_VOLTAGE 100u //In mV
//Battery voltage levels
#define BATTERY_FLAT       2370u //In mV
#define BATTERY_10_PERCENT 2430u //In mV
#define BATTERY_30_PERCENT 2500u //In mV
#define BATTERY_50_PERCENT 2600u //In mV
#define BATTERY_80_PERCENT 2700u //In mV

/*--------------------------------------------------------------Timings--------------------------------------------------------------*/
#define SCREEN_UPDATE_PERIOD 200u
#define GO_TO_SLEEP_TIMEOUT 180000u
#define BATTERY_VOLTAGE_MEASURING_PERIOD 60000u
#define TEMPERATURE_AND_HIMIDITY_MEASURING_PERIOD 30000u
#define MS_IN_1_MINUTE  60000ul 
#define SPEED_SET_TO_ZERO_TIMEOUT 1024u //In ms
#define SHORT_PRESS_TIME 4u
#define LONG_PRESS_TIME 2000u

/*--------------------------------------------------------------Machine logic constants--------------------------------------------------------------*/
#define MEASURING_WHEEL_PULSES_PER_METER 5u // 1m/COUNTER_STEP in 0.1m
#define COUNTER_STEP 2u //In 0.1m
#define MACHINE_SERVICE_INTERVALS 50000*10 //In m
#define MACHINE_SERVICE_WARNING_MESSAGE_SHOW_WHEN 1000*10 //In m
#define BROKEN_SENSOR_READING 0

#define FUNNY_PRESSES_SPEED_TO_FACTORY_RESET    100 //Need to press buttons every 50ms
#define FUNNY_PRESSES_TO_FACTORY_RESET  300u

/*--------------------------------------------------------------LCD--------------------------------------------------------------*/
#define LCD_FRAME_BUFFER_SIZE 1024u
#define NB_OF_SCREENS 5u
#define BACKLIGHT_BRIGHTNESS 255u

/*--------------------------------------------------------------ADC--------------------------------------------------------------*/
#define ADC_REF_VOLTAGE 3000u
#define ADC_MAX_VALUE    constexp(pow(2, 10)) //10-bit ADC, so 1024 
//Nb of ADC channels(Battery voltage on num1 and temperature sensor on num5)
#define ADC_NUMCHLS 2
#define NUM_ADC_SAMPLES 4 //Number of samples to average
#define ADC_BATTERY_CHANNEL 1
#define ADC_TEMPERATURE_CHANNEL 5

/*--------------------------------------------------------------GPIO's--------------------------------------------------------------*/

/*Battery charge enable*/
#define BATTERY_CHARGE_GPIO_PORT GPIOD
#define BATTERY_CHARGE_GPIO_NUM 7 
/*Boost Enable*/
#define BOOST_ENABLE_GPIO_PORT GPIOD //????????
#define BOOST_ENABLE_GPIO_NUM 0 //???????? 
/*Temperature sensor*/
#define TEMPERATURE_SENSOR_GPIO_PORT GPIOD
#define TEMPERATURE_SENSOR_GPIO_NUM 5 
/*Battery Voltage Sense*/
#define BATTERY_V_SENSE_GPIO_PORT GPIOA
#define BATTERY_V_SENSE_GPIO_NUM 1 
/*USB Voltage Sense*/
#define USB_V_SENSE_GPIO_PORT GPIOA
#define USB_V_SENSE_GPIO_NUM 2 
/*LCD*/
#define LCD_BACKLIGHT_GPIO_PORT GPIOD
#define LCD_BACKLIGHT_GPIO_NUM 2 
#define LCD_DI_GPIO_PORT GPIOC
#define LCD_DI_GPIO_NUM 6
#define LCD_CLK_GPIO_PORT GPIOC
#define LCD_CLK_GPIO_NUM 5
#define LCD_RESET_GPIO_PORT GPIOC
#define LCD_RESET_GPIO_NUM 3
#define LCD_DC_GPIO_PORT GPIOC
#define LCD_DC_GPIO_NUM 4 
/*Hall sensor inputs*/
#define HALL_INPUT_A_GPIO_PORT GPIOD
#define HALL_INPUT_A_GPIO_NUM 4
#define HALL_INPUT_B_GPIO_PORT GPIOD
#define HALL_INPUT_B_GPIO_NUM 3 
/*Button DOWN*/
#define BUTTON_DOWN_GPIO_PORT GPIOC
#define BUTTON_DOWN_GPIO_NUM 0
/*Button UP*/
#define BUTTON_UP_GPIO_PORT GPIOD
#define BUTTON_UP_GPIO_NUM 6 

/*--------------------------------------------------------------Variables--------------------------------------------------------------*/

extern uint16_t adcBuffer[NUM_ADC_SAMPLES*ADC_NUMCHLS];
extern uint32_t sysTickCnt;

extern uint8_t glcd_buffer[LCD_FRAME_BUFFER_SIZE];

extern uint32_t cntToZeroTheSpeedDisplay;
extern uint32_t cntToSleep;
extern uint32_t cntToMeasureBattery;
extern uint32_t cntToUpdateScreen;
/*--------------------------------------------------------------Exported functions--------------------------------------------------------------*/
extern void goToSleep (void);



 /**
* The NTC table has 33 interpolation points.
* Unit:1 °C
*
*/
const static int NTC_table[33] = {
  -54, -45, -36, -29, -24, -20, -17, -13, -10, 
  -8, -5, -2, 0, 3, 5, 8, 10, 13, 15, 18, 21, 
  24, 27, 31, 35, 39, 44, 49, 56, 66, 79, 104, 
  129
};


