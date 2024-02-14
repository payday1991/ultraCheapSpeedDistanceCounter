#include "ch32v003fun/ch32v003fun.h"
#include "include/main.h"
#include "include/init.h"
#include "include//machineData.h" 
#include "lcd/glcd.h" 
#include "lcd/visuals.h"

#include "include/aht20.h" 
#include "include/flash.h"
#include "include/adc.h"

/*Struct where we keep all the variables*/
machineData_t machineData;
mileageData_t mileageData;
uint32_t sysTickCnt;



/**
 * @brief Turn the LCD, backlight, HALL sensors off and go sleep.
 * 
 */
void goToSleep (void) {
	//Turn off the power(write 0 to the GPIO).
	BOOST_ENABLE_GPIO_PORT->BSHR = (1 << (16 + BOOST_ENABLE_GPIO_NUM));
	//Wait for death...
	while(1) //which is now unavoidable...
	{
		iwdgFeed(); //Feed the dog for the last time... :-(
	} 
}



/**
 * @brief Get the temperature from the NTC sensor to serve the battery charging.
 * 
 * @return int8_t 
 */
static int8_t getTemperature(void) {
	//Get the raw ADC reading(average of 4 samples) 
	uint16_t adcResult = adcGet(ADC_TEMPERATURE_CHANNEL); 
	// 	In the temperature range from -10°C to 50°C the error
	//  caused by the usage of a table is 1.378°C

	int16_t p1,p2; //Interpolating points
	/* Estimate the interpolating point before and after the ADC value. */
	p1 = NTC_table[ (adcResult >> 5)  ];
	p2 = NTC_table[ (adcResult >> 5)+1];
	
	/* Interpolate between both points. */
	return (int8_t)(p1 + ( (p2-p1) * (adcResult & 0x001F) ) / 32);
}



/**
 * @brief Takes the raw ADC value and decides how is the batery doing now...
 * 
 * @param machineData 
 */
static void checkBattery(machineData_t* machineData) {
	
	//Get the battery reading
	uint16_t adcResult = adcGet(ADC_BATTERY_CHANNEL);
  
	//Convert ADC Result to mV. ADC is 10-bit resolution.
	machineData->machine.batteryVoltage = (ADC_REF_VOLTAGE * adcResult)>>10;
	/*If battery is charging now, the voltage will be higher, so we have to compensate that to show the current state of charge correctly.*/
	if (machineData->flags.batteryCharging) 
	{
		if(machineData->machine.batteryVoltage < BATTERY_FLAT+BATTERY_CHARGING_OFFSET_VOLTAGE)  machineData->machine.batteryState = flat;
		else if (machineData->machine.batteryVoltage < BATTERY_10_PERCENT+BATTERY_CHARGING_OFFSET_VOLTAGE) machineData->machine.batteryState = tenPercent;
		else if (machineData->machine.batteryVoltage < BATTERY_30_PERCENT+BATTERY_CHARGING_OFFSET_VOLTAGE) machineData->machine.batteryState = thirtyPercent;
		else if (machineData->machine.batteryVoltage < BATTERY_50_PERCENT+BATTERY_CHARGING_OFFSET_VOLTAGE) machineData->machine.batteryState = fiftyPercent;
		else if (machineData->machine.batteryVoltage < BATTERY_80_PERCENT+BATTERY_CHARGING_OFFSET_VOLTAGE) machineData->machine.batteryState = eightyPercent; 
		else 
		{
			/*Tell everybody that the battery is full*/
			machineData->machine.batteryState = full;
			machineData->flags.batteryFullyCharged = true;
		}
	}
	else 
	{
		if(machineData->machine.batteryVoltage < BATTERY_FLAT) machineData->machine.batteryState = flat;
		else if (machineData->machine.batteryVoltage < BATTERY_10_PERCENT) machineData->machine.batteryState = tenPercent;
		else if (machineData->machine.batteryVoltage < BATTERY_30_PERCENT) machineData->machine.batteryState = thirtyPercent;
		else if (machineData->machine.batteryVoltage < BATTERY_50_PERCENT) machineData->machine.batteryState = fiftyPercent;
		else if (machineData->machine.batteryVoltage < BATTERY_80_PERCENT) machineData->machine.batteryState = eightyPercent;
		else machineData->machine.batteryState = full; 
	}
	/*Reset the Battery fully charged bit if battery is not full*/
	if (machineData->machine.batteryState !=full)
	{
		/*Tell state machine that the battery is NOT full*/
		machineData->flags.batteryFullyCharged = false;
	}
}

#if defined(USE_TEMPERATURE_HUMIDITY_SENSOR)
/**
 * @brief Initializes and reads the sensor data.
 * 
 * This function initializes the sensor and reads the outside temperature and humidity.
 * If the sensor initialization or reading fails, the function sets the values to a predefined broken sensor reading.
 */
static void initializeAndReadTheSensor() {
	if (aht20init() || aht20read(&machineData.machine.outsideTemperature, &machineData.machine.outsideHumidity)) {
		machineData.machine.outsideTemperature = BROKEN_SENSOR_READING;
		machineData.machine.outsideHumidity = BROKEN_SENSOR_READING;
	}
}
#endif

/**
 * @brief Updates the current screen being displayed.
 * 
 * This function sets the current screen of the machine to the specified screen.
 * It then calls the updateScreen function to update the visuals on the screen.
 * 
 * @param screen The screen to be displayed.
 */
static void showScreen(currentScreen_e screen) {
	machineData.visuals.currentScreen = screen;
	updateScreen(&machineData); 
}


static inline void displayInitialScreen() {
    if (machineData.machine.batteryState == flat) {
        showScreen(lowBatteryScreen);
        goToSleep();
    } else if (machineData.flags.needsServicing) {
        showScreen(serviceMeScreen);
    } else {
        showScreen(logoScreen); 
    }
}

static inline void initializeSystem() {
    init();
    checkBattery(&machineData);
    machineData.machine.batteryTemperature = getTemperature();
    getSavedMileageDataFromFlash(FLASH_ADDR_TO_STORE_BACKUP_DATA, NON_VOLATILE_FLASH_DATA_STORAGE_SIZE, sizeof(mileageData));

    #if defined(USE_TEMPERATURE_HUMIDITY_SENSOR)
	initializeAndReadTheSensor();
	#endif

    displayInitialScreen();
    systick_init();
}



void mainLoop() {
    while (1) {

        if (machineData.flags.batteryNeedsMeasuring)
		{
			checkBattery(&machineData);
			machineData.machine.batteryTemperature = getTemperature(); //Get the temperature as well
			machineData.flags.batteryNeedsMeasuring = false;
		}

		#if defined(USE_TEMPERATURE_HUMIDITY_SENSOR)
		if (machineData.flags.temperatureAndHumidityNeedsMeasuring)
		{
			if(aht20read(&machineData.machine.outsideTemperature,&machineData.machine.outsideHumidity)) 
			{
				//If the sensor is not connected, set the values to 0
				machineData.machine.outsideTemperature = BROKEN_SENSOR_READING;
				machineData.machine.outsideHumidity = BROKEN_SENSOR_READING;
			}
			machineData.flags.temperatureAndHumidityNeedsMeasuring = false; 
		}
		#endif

		if (machineData.flags.screenNeedsUpdating) 
		{
			iwdgFeed(); //Feed the watchdog
			updateScreen(&machineData);
			machineData.flags.screenNeedsUpdating = false;
		}

		/*Check if there is a charger connected flag been set*/
		if (machineData.flags.batteryCharging)
		{
			/*Turn the charger on*/
			GPIO_digitalWrite_0(GPIOv_from_PORT_PIN(GPIO_port_D, BATTERY_CHARGE_GPIO_NUM));
		}
		/*No USB voltage present, turn off*/
		else GPIO_digitalWrite_1(GPIOv_from_PORT_PIN(GPIO_port_D, BATTERY_CHARGE_GPIO_NUM));
    }
}

int main(void) {
    initializeSystem();
    mainLoop();
}
