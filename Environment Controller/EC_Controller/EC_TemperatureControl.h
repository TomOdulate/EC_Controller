// EC_TemperatureControl.h
/******************************************************************************
	NAME   : EC_TemperatureControl.h
	Version: 0.1
	Created: 01/10/2013
	Author : Thomas Odulate.
	NOTES  : Provides temperature control by calculating duty cycles for two 
			fans (Intake & Exhaust).  For heating control, it simply sets a 
			flag variable that can then be used to turn a heater on or off.

******************************************************************************/

#ifndef _EC_TEMPERATURECONTROL_h
#define _EC_TEMPERATURECONTROL_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

class EC_TemperatureControl
{
	private:
		float targetCoolingTemp;		// Used to track the target temperature to cool to when cooling.
		float targetHeatingTemp;		// Used to track the target temperature to heat to when heating.		
	public:
		EC_TemperatureControl();		// Constructor
	protected:
		short inFanSpeed;				// Holds the calculated duty cycle for the intake fan
		short outFanSpeed;				// Holds the calculated duty cycle for the exhaust fan
		bool cooling;					// Flag indicates that the max temperature has been hit
		bool heating;					// Flag indicates that the min temperature has been hit
		bool InFanState, outFanState;	// Flag used to track the current state of the fans.

		// Uses current temperature to calculate required fan rate
		short CalculateFanRate(float currentTemp , short tempMin, short tempMax, short hyst);
	
		// Uses the calculated fan rate to set fan speeds.
		void ControlFanSpeed( float currentTemp, short tempMin, short tempMax, float hyst, float NegFBack, 
			short Fan1Min, short Fan1Max, short Fan2Mmin, short Fan2Max);	 	
};

//extern EC_TemperatureControl EC_TEMPERATURECONTROL;

#endif

