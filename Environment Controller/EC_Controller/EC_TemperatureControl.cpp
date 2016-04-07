// 
/******************************************************************************
	NAME   : EC_TemperatureControl.cpp
	Version: 0.1
	Created: 01/10/2013
	Author : Thomas Odulate.
	NOTES  : Provides temperature control by calculating duty cycles for two 
			fans (Intake & Exhaust).  For heating control, it simply sets a 
			flag variable that can then be used to turn a heater on or off.

******************************************************************************/ 

#include "EC_TemperatureControl.h"

//EC_TemperatureControl EC_TEMPERATURECONTROL;
/* 
	Constructor
*/
EC_TemperatureControl::EC_TemperatureControl()
{
	this->cooling			= false;
	this->heating			= false;
	this->targetCoolingTemp = 0.0f;
	this->targetHeatingTemp = 0.0f;
	this->InFanState		= false;
	this->outFanState		= false;	
	this->inFanSpeed		= 0;
	this->outFanSpeed		= 0;

	#if defined(DEBUG_FAN_CONTROLLER)
		Serial.println("Fan controller Initialised.");
	#endif
}
/*
	Maintains the fan speeds according to current
	temperature ranges.
*/
void EC_TemperatureControl::ControlFanSpeed(float currentTemp, short tempMin, short tempMax, float hyst, 
											float NegFBack, short Fan1Min, short Fan1Max, short Fan2Min, short Fan2Max)
{
	int fanRate = CalculateFanRate(currentTemp, tempMin, tempMax, hyst);
	outFanSpeed = fanRate;

	// Enforce Min / Max speeds for outtake fan (PWM2)
	if(outFanSpeed < map(Fan2Min,0,100,0,255))
		outFanSpeed = map(Fan2Min,0,100,0,255);
	else 
		if(outFanSpeed >= map(Fan2Max,0,100,0,255))
			outFanSpeed = map(Fan2Max,0,100,0,255);
	
	inFanSpeed = (NegFBack)?outFanSpeed - ((NegFBack/100)*255) : fanRate;	

	// Enforce Min / Max speeds for intake fan (PWM1)
	int maxIntakeSpeed = (NegFBack)?outFanSpeed - ((NegFBack/100)*outFanSpeed)
											:map(Fan1Max,0,100,0,255);
	if(inFanSpeed >= maxIntakeSpeed)
		inFanSpeed = maxIntakeSpeed;
	else
	{
		if(inFanSpeed <= map(Fan1Min,0,100,0,255))
		{
			// Calculated speed is less than the allowed min for  intake fan, so 
			// need to increase speed of output to maintain negative feedback.
			inFanSpeed = map(Fan1Min,0,100,0,255);
			if(NegFBack>0 & inFanSpeed>0)
			{
				outFanSpeed = inFanSpeed + ((NegFBack/100)*255);	
			}
		}
	}
}
/*
	Returns a calculated fan rate (duty cycle) based on the current temperature.  Also sets the heating
	& cooling flags.
*/
short EC_TemperatureControl::CalculateFanRate(float currentTemp, short tempMin, short tempMax, short hyst )
{
	int fanRate = 0;

	// Are we within allowed temperature  bounds?
	
	// Is it too hot?
	if(currentTemp >= tempMax)
	{
		cooling = true;
		targetCoolingTemp = tempMax - hyst;
	}
	
	// Is it too cold?
	if(currentTemp <= tempMin)
	{
		heating = true;
		targetHeatingTemp = tempMin + hyst;
	}

	// If we're currently heating, check if we've reached out target
	// temperature, if so set flag to stop heating.
	if(heating)
	{
		if(currentTemp >= targetHeatingTemp)
		{
			// Turn the heater output off
			heating = false;
		}
	}

	if(cooling)
	{
		// set fan rate according to how far from target 
		// temp we currently are.

		// We've reached our target cooling temp
		if( currentTemp <= targetCoolingTemp )
		{
			fanRate = 0;
			cooling = false;
		}
		else	// If we haven't cooled enough yet, so just re-adjust fanspeed.
		{
			float a = (currentTemp - targetCoolingTemp);
			if(a >= hyst)
				fanRate = 255;
			else 
			{
				a *= 100;
				fanRate = map((int)a, 0, 190,0,255);
			}
		}
	}
	return fanRate;
}