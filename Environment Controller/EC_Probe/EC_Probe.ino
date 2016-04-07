/****************************************************************************** 
	NAME   : EC_Probe.ino
	Version: 1
	Created: 28/05/2012
	Author : Thomas Odulate
	NOTES  : Firmware for the temperature and humidity probe of the 
			 Environmental Controller project.

	
	The EC_Probe firmware provides the following control mechinisms for the 
	circuit; 
		1. Measuring & calculating relative humidity, temperature & light levels. 
		2. Storing those values ready for transmission.
		3. Listening for requests via RS232 (via RS485) from the master.
		4. Responding to requests from the master.
	
	The probe uses an the Modbus protocol.  Using this protocol, even though its 
	implementation does not completely meet the full Modbus specification is 
	a great way to get an industry accepted protocol in place.	
	
	For the implementation I've used SimpleModbus (available on GitHub).  This 
	implementation only supports the Modbus commands 3 & 16, read registers and 
	Write Multiple Registers (refer to Modbus specification for details).  This 
	is all that is required for the probe to function.

******************************************************************************/

#include <Arduino.h>
#include "EC_Probe_Types.h"
#include "SimpleModbusSlaveSoftwareSerial.h"
#include "SoftwareSerial.h"


void setup()
{
	// Setup the microcontroller input / output pins.
	pinMode(T_SENSOR_PIN, INPUT);		// Temperature
	pinMode(H_SENSOR_PIN, INPUT);		// Humidity
	pinMode(L_SENSOR_PIN, INPUT);		// Light sensor
	pinMode(RS232_RTS,OUTPUT);			// RS232 control pin

	/*	Setup the modbus library....
			
			parameters(	long baudrate, 
						unsigned char ID, 
						unsigned char transmit enable pin, 
						unsigned int holding registers size )
                
     The transmit enable pin is used in half duplex communication to activate a
	 MAX485 or similar to deactivate this mode use any value < 2 because 0 & 1
	 is reserved for Rx & Tx
  */  
	
	modbus_configure(MODBUS_BAUDRATE, PROBE_ADDRESS, RS232_RTS, TOTAL_REGS_SIZE);  
}

void loop()
{	
	//	Take sensor readings...
	float currentTemp		= ReadTemperature();
	float currentHumidity	= ReadHumidity();

	// Store the results...
	holdingRegs[TMP1]		= currentTemp;
	holdingRegs[TMP2]		= GetFractionalPart( currentTemp );
	holdingRegs[HUM1]		= currentHumidity;
	holdingRegs[HUM2]		= GetFractionalPart( currentHumidity );
	holdingRegs[LIGHT]		= ReadLightLevel();
	holdingRegs[VERSION]	= PROBE_VERSION;
	
	// Peform any network communication.
	modbus_update(holdingRegs);
}

float ReadTemperature(void)
{
	/*	NB From Datasheet...
		The TMP35 is functionally compatible with the LM35/LM45 and provides a 250 mV output at 25°C. The TMP35 reads
		temperatures from 10°C to 125°C. The TMP36 is specified from −40°C to +125°C, provides a 750 mV output at 25°C, and
		operates to 125°C from a single 2.7 V supply. The TMP36 is functionally compatible with the LM50. Both the TMP35 and
		TMP36 have an output scale factor of 10 mV/°C
	*/
	float temperature;
	float avgTemp = 0;

	// Take a number of samples from sensor pin.
	for(int i =0;i<MAX_TEMP_SAMPLES;i++)
	{
		temperature = GetVoltage(T_SENSOR_PIN);		// Get the voltage reading from the temperature pin
		temperature = ((temperature - .580f) * 100);	// Convert from 10 mv per degree with 500 mV offset to degrees ((volatge - 500mV) times 100)
		//temperature = ((temperature - .750f) * 100);	// Convert from 10 mv per degree with 750 mV offset to degrees ((volatge - 750mV) times 100)
		avgTemp += temperature;						// Sum the values for an average calculation.
		delay(SAMPLE_DELAY);
	}
	
	// return the average.
	return (avgTemp / MAX_TEMP_SAMPLES);			// return the average. 
}

float ReadHumidity(void)
{
	/* Formula from datasheet.... Given that,
			Zero offset = 0.826v
			Slope = 0.0315
		
		RH = (sensor output voltage - zero offset) / slope
		  or RH = (Vout - 0.826) / 0.0315
	*/

	float total = 0;
	float zeroOffset = 0.826;
	float slope = 0.0315;

	for(int i =0;i<MAX_HUM_SAMPLES;i++)
	{
		float vOut = GetVoltage(H_SENSOR_PIN);
		total += ((vOut - zeroOffset) / slope);		
	}	

	return ((float)(total / MAX_HUM_SAMPLES));
}

static float GetVoltage(int pin)		   // Returns the voltage on the analog input defined by pin
{
	return (analogRead(pin) * .004882814); // Convert from a 0 to 1023 digital range to 0 to 5 volts (each 1 reading equals ~ 5 millivolts)
	//return (analogRead(pin) * 0.001953125);
};

int ReadLightLevel(void)
{
	int lightLevel = 0;
	lightLevel = analogRead( L_SENSOR_PIN );
	delay(1);	
	return lightLevel;
}