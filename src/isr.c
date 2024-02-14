#include "include/main.h"
#include "include/machineData.h"
#include "include/flash.h"

uint32_t cntToZeroTheSpeedDisplay = SPEED_SET_TO_ZERO_TIMEOUT;
uint32_t cntToSleep = GO_TO_SLEEP_TIMEOUT;
uint32_t cntToMeasureBattery = BATTERY_VOLTAGE_MEASURING_PERIOD;
uint32_t cntToMeasureTemperatureAndHumidity = TEMPERATURE_AND_HIMIDITY_MEASURING_PERIOD;
uint32_t cntToUpdateScreen = SCREEN_UPDATE_PERIOD;




/**
 * @brief Interrupt service routine for the encoder.
 * 
 * This function is the interrupt service routine for encoder interrupt. 
 * The function updates the mileage data, calculates the machine speed, and resets the timeout values.
 * 
 * @param None
 * @return None
 */
__attribute__((interrupt("WCH-Interrupt-fast"))) void EXTI7_0_IRQHandler( void ) { 
	
	static uint32_t prvSysTickcnt;
	uint32_t sysTickCntDiff = sysTickCnt - prvSysTickcnt; 
	
	/*Add to the mileage in any case*/
	mileageData.machineMileage += COUNTER_STEP;
	//Subtract from the service me counter
	mileageData.serviceOverdue -= COUNTER_STEP;

	/*Check the interrupt source. If it's an ENC_A, then go on*/ 
	/*We got just 1 interrupt source for now, so we can save a few microseconds...*/ 
	// if (EXTI->INTFR && 1<<HALL_INPUT_A_GPIO_NUM)
	// { 
		if (!GPIO_digitalRead(GPIOv_from_PORT_PIN(GPIO_port_D, HALL_INPUT_B_GPIO_NUM))) 
		{
			/*Forwards*/
			machineData.machine.currentDistance += COUNTER_STEP; //Measuring wheel round length is 0.1m.
			/*Calculate speed*/
			machineData.machine.speed = (MS_IN_1_MINUTE/MEASURING_WHEEL_PULSES_PER_METER) / sysTickCntDiff;
		}
		else 
		{
			/*If more than 0*/
			if (machineData.machine.currentDistance) machineData.machine.currentDistance -= COUNTER_STEP;
			/*Calculate speed and add negative sign to it*/
			machineData.machine.speed = -((MS_IN_1_MINUTE/MEASURING_WHEEL_PULSES_PER_METER) / sysTickCntDiff );
		}

		//Reset the timeout to prevent zeroing the speed.
		cntToZeroTheSpeedDisplay = SPEED_SET_TO_ZERO_TIMEOUT;
		/*We definately don't want to sleep while doing the job...*/
		cntToSleep = GO_TO_SLEEP_TIMEOUT;
		/*Save the timestamp for the next calculation*/
		prvSysTickcnt=sysTickCnt;
		/*Clear the interrupt flag*/
		EXTI->INTFR = 1<<HALL_INPUT_A_GPIO_NUM;
	// } 
}



/**
 * @brief Increments the time counter and updates related variables.
 *
 * This function is responsible for incrementing the time counter and updating
 * the machine's on-time age. It is called periodically by the interrupt service routine.
 * The time counter is incremented every minute, and when it reaches zero, the machine's
 * time is incremented and the on-time age is updated. The localSysTickCnt variable is
 * used to keep track of the remaining time until the next increment.
 */
static void incrementTimeCounter() {
    static uint32_t localSysTickCnt = MS_IN_1_MINUTE; 
    if (--localSysTickCnt == 0) {
        machineData.machine.time++;
        mileageData.machineOnTimeAge++;
        localSysTickCnt = MS_IN_1_MINUTE;
    }
}



/**
 * @brief Zeroes the speed display if no signal is received within a certain timeout period(1024ms).
 * 
 * This function is responsible for checking if a signal has been received within a certain timeout period.
 * If no signal is received, it sets the machine speed to zero and resets the timeout counter.
 */
static void zeroSpeedIfNoSignal() {
    if (--cntToZeroTheSpeedDisplay == 0) {
        machineData.machine.speed = 0; 
        cntToZeroTheSpeedDisplay = SPEED_SET_TO_ZERO_TIMEOUT;
    }
}



/**
 * @brief Checks the battery charging status.
 * 
 * This function checks if the charger has been plugged in and determines if the battery is currently charging.
 * If the battery is charging and it is not fully charged, it sets the appropriate flags and prevents the system from going to sleep.
 * If the charger is not plugged in, it sets the batteryCharging flag to false.
 */
static void checkBatteryChargingStatus() {
    /*Check if the charger been plugged in*/
    if (GPIO_digitalRead(GPIOv_from_PORT_PIN(GPIO_port_A, USB_V_SENSE_GPIO_NUM))) //Check if battery is charging now
    {
        if (machineData.flags.batteryFullyCharged == false)
        {
            machineData.flags.batteryNeedsMeasuring = true;
            machineData.flags.batteryCharging = true;
            //Prevent from going to sleep while on charging
            cntToSleep = GO_TO_SLEEP_TIMEOUT;
        }
    }
    else 
    {
        machineData.flags.batteryCharging = false;
    }
}



/**
 * @brief Updates the status of the backlight based on the machineData flags.
 *        If the backlightOnRq flag is set, the backlight brightness is increased until it reaches the maximum brightness defined by BACKLIGHT_BRIGHTNESS.
 *        If the backlightOffRq flag is set, the backlight brightness is decreased until it reaches 0.
 */
static void updateBacklightStatus() {
    /*Do the backlight*/
    if (machineData.flags.backlightOnRq)
    {
        if (TIM1->CH1CVR < BACKLIGHT_BRIGHTNESS)
        {
            TIM1->CH1CVR++;
        }
        else machineData.flags.backlightOnRq = false;
    }
    if (machineData.flags.backlightOffRq)
    {
        if (TIM1->CH1CVR > 0)
        {
            TIM1->CH1CVR--;
        }
        else machineData.flags.backlightOffRq = false;
    }
}



/**
 * @brief Handles the functionality of the down button.
 * 
 * This function checks if the down button has been pressed or depressed and performs the corresponding actions.
 * If the button is pressed for a long duration, it resets the machine's current distance and time.
 * If the button is pressed for a short duration, it changes the screen to the next one in the sequence.
 * 
 * @note This function assumes that the GPIO pin for the down button has been configured correctly.
 */
static void handleDownButton() {
    static _Bool DOWNbeenPressed;
    static uint32_t oldDOWNtimeStamp; //to determine the long and short press.
	//Check DOWN button
	if(GPIO_digitalRead(GPIOv_from_PORT_PIN(GPIO_port_C, BUTTON_DOWN_GPIO_NUM))) { //been pressed
		if (!DOWNbeenPressed) 
		{
			DOWNbeenPressed = true;
			oldDOWNtimeStamp = sysTickCnt;
		}
		//Check if it was a long press
		if ((sysTickCnt - oldDOWNtimeStamp) > LONG_PRESS_TIME) //Long press. Reset the counter
		{
			machineData.machine.currentDistance = 0;
			machineData.machine.time = 0;
		} 
	}
	else if (DOWNbeenPressed) //been depressed
	{ 
		if ((sysTickCnt - oldDOWNtimeStamp) > SHORT_PRESS_TIME) //Short press. Change the screen
		{ 
			machineData.visuals.currentScreen++;
			if (machineData.visuals.currentScreen == NB_OF_SCREENS) 
			{
				machineData.visuals.currentScreen = mainScreenDistance;
			}
		}
		DOWNbeenPressed = 0;
		cntToSleep = GO_TO_SLEEP_TIMEOUT;
	}
}



/**
 * @brief Handles the functionality for the UP button press.
 *
 * This function checks if the UP button has been pressed or depressed and performs the corresponding actions.
 * If the button is pressed for a long duration, it toggles the backlight on or off.
 * If the button is pressed for a short duration, it toggles the backlight on or off.
 * The function also updates the machineData flags accordingly.
 * 
 * @note This function assumes that the GPIO pin for the UP button has been configured correctly.
 * 
 * @note The LONG_PRESS_TIME and SHORT_PRESS_TIME constants should be defined appropriately before using this function.
 * 
 * @note The sysTickCnt and cntToSleep variables should be defined and updated externally.
 */
static void handleUpButton() {
     static _Bool UPbeenPressed; 
	static uint32_t oldUPtimeStamp; //to determine the long and short press.
	/*Check if the UP button been pressed*/
	if(GPIO_digitalRead(GPIOv_from_PORT_PIN(GPIO_port_D, BUTTON_UP_GPIO_NUM))) {//been pressed
				if (!UPbeenPressed) {
					UPbeenPressed = 1;
					oldUPtimeStamp = sysTickCnt;
				}
	}
	else if (UPbeenPressed) { //been depressed
		if ((sysTickCnt - oldUPtimeStamp) > LONG_PRESS_TIME) {//Long press. Toggle the Backlight
				if (TIM1->CH1CVR != 0)
				{ 
					machineData.flags.backlightOffRq = true;
					machineData.flags.backlightOnRq = false;
				}
				else
				{ 
					machineData.flags.backlightOnRq = true;
					machineData.flags.backlightOffRq = false;
				}
			}
		else if ((sysTickCnt - oldUPtimeStamp) > SHORT_PRESS_TIME) {//Short press. Toggle the Backlight
			/*If backlight is off already, set the ON request flag and vice-versa*/
			if (TIM1->CH1CVR != 0)
				{
					machineData.flags.backlightOffRq = true;
					machineData.flags.backlightOnRq = false;
				}
				else
				{
					machineData.flags.backlightOnRq = true;
					machineData.flags.backlightOffRq = false;
				}
		}
		UPbeenPressed = 0;
		cntToSleep = GO_TO_SLEEP_TIMEOUT;
	}
}



/**
 * @brief Checks for the funny factory reset button pressing sequence.
 * 
 * This function checks if the BACKLIGHT(UP) button is pressed in a specific sequence
 * to trigger a factory reset. It keeps track of the number of presses and updates
 * the serviceOverdue variable in the mileageData structure when the sequence is completed.
 * 
 * @note This function assumes the availability of GPIO_digitalRead() function and the
 * GPIOv_from_PORT_PIN() macro to read the state of the buttons.
 */
static void checkFunnyButtonSequence() {
    //Check BACKLIGHT(UP) button
	/*Check for the funny factory reset button pressing sequence*/
	static uint16_t cntFunny=FUNNY_PRESSES_TO_FACTORY_RESET;
	if (GPIO_digitalRead(GPIOv_from_PORT_PIN(GPIO_port_C, BUTTON_DOWN_GPIO_NUM)))
	{
		static bool prvButtonState = false;
		if (prvButtonState)
		{
			if (GPIO_digitalRead(GPIOv_from_PORT_PIN(GPIO_port_D, BUTTON_UP_GPIO_NUM)))
			{
				cntFunny--;
				if (!cntFunny) mileageData.serviceOverdue = MACHINE_SERVICE_INTERVALS; 
				prvButtonState=!prvButtonState;
			}
		}
		else
		{
			if (!GPIO_digitalRead(GPIOv_from_PORT_PIN(GPIO_port_D, BUTTON_UP_GPIO_NUM)))
			{
				cntFunny--;
				if (!cntFunny) mileageData.serviceOverdue = MACHINE_SERVICE_INTERVALS; 
				prvButtonState=!prvButtonState;
			}
		}
	}
	else cntFunny=FUNNY_PRESSES_TO_FACTORY_RESET;
}



/**
 * @brief Handles button presses.
 * 
 * This function calls the necessary functions to handle the down button, up button, and check for a funny button sequence.
 */
static void handleButtonPresses() {
    handleDownButton();
    handleUpButton();
    checkFunnyButtonSequence();
}



/**
 * @brief Performs periodic measurements and updates flags accordingly.
 * 
 * This function checks if it is time to measure the battery voltage, temperature and humidity,
 * and update the screen. If it is time, it sets the corresponding flags in the machineData structure.
 * 
 * @note This function assumes that the variables cntToMeasureBattery, cntToMeasureTemperatureAndHumidity,
 * and cntToUpdateScreen are properly initialized with the desired measuring periods.
 */
static void periodicMeasurements() {
    //Check if we need to measure the battery voltage
    if (--cntToMeasureBattery == 0) {
        machineData.flags.batteryNeedsMeasuring = 1;
        cntToMeasureBattery = BATTERY_VOLTAGE_MEASURING_PERIOD;
    }
    //Check if we need to measure the temperature and humidity
    if (--cntToMeasureTemperatureAndHumidity == 0) {
        machineData.flags.temperatureAndHumidityNeedsMeasuring = 1;
        cntToMeasureTemperatureAndHumidity = TEMPERATURE_AND_HIMIDITY_MEASURING_PERIOD;
    } 
    //Check if we need to update the screen
    if (--cntToUpdateScreen == 0 && !machineData.flags.screenNeedsUpdating) {
        machineData.flags.screenNeedsUpdating = 1;
        cntToUpdateScreen = SCREEN_UPDATE_PERIOD;
    }
}



/**
 * @brief Checks if we want to power-off.
 * 
 * This function is called to determine if the system should power-off itself.
 * It decrements the `cntToSleep` variable and if it reaches zero, it saves the machine mileage data to flash memory
 * and disconnects the battery.
 */
static void doWeWantSleep() { 
    if (--cntToSleep == 0) {
        saveMachineMileageDataToFlash(FLASH_ADDR_TO_STORE_BACKUP_DATA, NON_VOLATILE_FLASH_DATA_STORAGE_SIZE, sizeof(mileageData));
        goToSleep();
    }
}



/**
 * @brief Interrupt handler for the SysTick timer.
 * 
 * This function is called when the SysTick timer interrupt occurs.
 * It performs various tasks such as incrementing the time counter,
 * checking battery charging status, updating backlight status, handling
 * button presses, and performing periodic measurements. It also updates
 * the SysTick counter and clears the interrupt flag.
 */
__attribute__((interrupt("WCH-Interrupt-fast"))) void SysTick_Handler(void) { 
    incrementTimeCounter();
    zeroSpeedIfNoSignal();
    checkBatteryChargingStatus();
    updateBacklightStatus();
    handleButtonPresses();
    periodicMeasurements();
    doWeWantSleep();
    
    /* update counter */
	sysTickCnt++;
	SysTick->CMP += (FUNCONF_SYSTEM_CORE_CLOCK/1000); 
	/* clear IRQ */
	SysTick->SR = 0; 

}

