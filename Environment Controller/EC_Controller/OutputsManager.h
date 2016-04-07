// OutputsManager.h

#ifndef _OUTPUTSMANAGER_h
#define _OUTPUTSMANAGER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include "SDLogger.h"
#include "Debug.h"
#include "EC_TemperatureControl.h"

#define NUM_OUTPUTS 6
#define NUM_PWM_OUTPUTS 2

enum FANS {INTAKE_FAN,EXHAUST_FAN};

class OutputsManager : public EC_TemperatureControl
{
	private:
		bool fanSpeedHasChanged;					// Flag indicates a change in Fan speed
		bool fanStateHasChanged;					// Flag indicates a change in Fan state (on / off)
		int _outputPins[NUM_OUTPUTS];				// Holds the output pin definitions
		bool _outputPinsState[NUM_OUTPUTS];			// Holds the outputs current state i.e on / off 
		uint8_t _PWMOutputPins[NUM_PWM_OUTPUTS];	// Holds the PWM output pin definitions
		short _currentDutyCycle[NUM_PWM_OUTPUTS];	// Holds the current PWM duty cycle for PWM outputs
		SDLogger *logger;							// Reference to the logging object to log output changes
		bool InFanState;							// Used to track the last state of the intake fan.
		bool outFanState;							// Used to track the last state of the outtake fan.	
		void SetPWMOutputValue(int PWMOutputPin, short dutyCycle);	// Sets the duty cycle of a PWM output	

	public:
		// Constructor
		OutputsManager(uint8_t OutPut1Pin, uint8_t OutPut2Pin, uint8_t OutPut3Pin, uint8_t OutPut4Pin,
		uint8_t OutPut5Pin, uint8_t OutPut6Pin, uint8_t OutPutPWM1Pin, uint8_t OutPutPWM2Pin, SDLogger* );
	
		bool GetState(uint8_t outputNumber);			// Returns the current state of a single output
		byte GetState();								// Returns the current state of all outputs
		byte SetState(byte);							// Sets the state of all outputs
		bool SetState(uint8_t outputNumber, bool on);	// Sets the state of a single output
		void Test();									// Cycles all outputs on / off.

		// Calculates required fan speeds and sets them
		void MaintainTemperature(float currentTemp, short tempMin, short tempMax, short hyst, float NegFBack, 
			short Fan1Min, short Fan1Max, short Fan2Mmin, short Fan2Max);
};

#endif
