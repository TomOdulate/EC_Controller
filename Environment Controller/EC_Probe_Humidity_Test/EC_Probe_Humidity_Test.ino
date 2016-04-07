
#include "Arduino.h"



void setup()
{

	Serial.begin(4800);

}

void loop()
{

  /* add main program code here */
	unsigned long humidity= ReadHumidity();
	Serial.println(humidity);
	delay(1000);
}


unsigned long ReadHumidity(void)
{
	
	//return RH(GetAverageFromSamples());  
	return(GetAverageFromSamples());
}

static float GetVoltage(int pin)		   // Returns the voltage on the analog input defined by pin
{
	return (analogRead(pin) * .004882814); // Convert from a 0 to 1023 digital range to 0 to 5 volts (each 1 reading equals ~ 5 millivolts
};

unsigned long GetAverageFromSamples()
{
	unsigned long averageTime = 0;					// Temp store for the calculated average discharge time.
	
	for (int i=0; i<MAX_HUM_SAMPLES ;i++){			// Take x number of samples by...
		averageTime += RCtime( H_SENSOR_PIN );		// Calling RCTime function.
		delay(SAMPLE_DELAY);						// <--- not sure i need this! 		
	}
	averageTime = ( averageTime/MAX_HUM_SAMPLES );	// Calculate an average from all the samples taken.
	return( averageTime );							// Return the average. NB Unsigned long to long conversions! ;)
}



unsigned long RCtime(int sensPin)
{
	pinMode(sensPin, OUTPUT);			// Make pin OUTPUT
	digitalWrite(sensPin, LOW);		// Make pin HIGH to discharge capacitor - study the schematic
	delay(10);							// Wait a  ms to make sure cap is discharged
	pinMode(sensPin, INPUT);			// Turn pin into an input and time till pin goes low
	digitalWrite(sensPin, HIGH);			// Turn pullups off - or it won't work
	unsigned long startTime = micros(); // Record the startTime
	
	/*	Wait for pin to go low. MAY HAVE TO PUT A TIMER ON THIS, IT COULD WAIT FOREVER! 
		Also note that if the SAMPLE_DELAY is set too high, then you'll never get the 
		true capacitance discharge time, since that call to delay() blocks, i.e is may 
		discharge while its waiting.  Therefore a smaller wait time ( <10ms ) should be 
		used. */

	while(digitalRead(sensPin))			
		//delay(0);	
	
	return (micros()-startTime);	
}