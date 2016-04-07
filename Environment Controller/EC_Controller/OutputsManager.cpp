#include "OutputsManager.h"
/*
	Initialises the outputs.  Sets them all off, and PWM duty cycles are set at 0.
*/
OutputsManager::OutputsManager(uint8_t OutPut1Pin, uint8_t OutPut2Pin, uint8_t OutPut3Pin, uint8_t OutPut4Pin,
				uint8_t OutPut5Pin, uint8_t OutPut6Pin, uint8_t OutPutPWM1Pin, uint8_t OutPutPWM2Pin, SDLogger* log ) : EC_TemperatureControl()
{
	// Initialise base class
	//EC_TEMPERATURECONTROL.init();

	// Store a reference to the logging object
	this->logger = log;
	
	// Setup the output pins definitions
	this->_outputPins[0]	= OutPut1Pin;
	this->_outputPins[1]	= OutPut2Pin;
	this->_outputPins[2]	= OutPut3Pin;
	this->_outputPins[3]	= OutPut4Pin;
	this->_outputPins[4]	= OutPut5Pin;
	this->_outputPins[5]	= OutPut6Pin;
	this->_PWMOutputPins[0] = OutPutPWM1Pin;
	this->_PWMOutputPins[1] = OutPutPWM2Pin;
	
	// Ensure all outputs are set as outputs
	for(int output = 0;output<NUM_OUTPUTS;output++)
	{
		pinMode(this->_outputPins[output], OUTPUT);	
		_outputPinsState[output] = false;
	}
	pinMode(OutPutPWM1Pin, OUTPUT); 
	pinMode(OutPutPWM2Pin, OUTPUT); 
	
	// Reset the fanstate flags
	this->InFanState	= false;
	this->outFanState	= false;

	// Ensure outputs are all off
	this->SetState(B000000);	
}
/*
	Returns the current state of all outputs as binary.
	Note the two MSB's are ignored.

	E.g a return value of B00000111 means outputs 1-3 are on and 4-6 are off 'b'
*/
byte OutputsManager::GetState()
{
	byte b = B00000000;
	for(int output = 0; output<NUM_OUTPUTS; output++)
	{
		if( this->_outputPinsState[output] == true)
			b |= (1 << output);
	}
	return b;
}
/*
	Sets the state (on / off) of all outputs, then reads their state
	and returns it as a byte. Note the two MSB's are ignored.

	E.g to set outputs 1-3 on and 4-6 off 'b' would equal = B00000111;
*/
byte OutputsManager::SetState(byte b)
{
	for(int output = 0; output<NUM_OUTPUTS; output++)
		this->SetState( output,(b & (1<<output)) );
	
	return this->GetState();
}
/*
	Set the state (on / off) of a single output, then reads its state
	and returns it.

	N.B the parameter outputNumber references the output number NOT
	the acutal output pin.  The actual pin for each output is defined 
	in the _outputPins[] array.
*/
bool OutputsManager::SetState( uint8_t outputNumber, bool onOff )
{
	#if defined(DEBUG_OUTPUTS)
		Serial.print("Setting output: ");
		Serial.print(outputNumber);
		Serial.print( (onOff)?" HIGH.":" LOW."); 
		Serial.print(" (pin ");
		Serial.print(_outputPins[outputNumber]);
		Serial.println(")");
	#endif
	this->_outputPinsState[outputNumber] = onOff;
	digitalWrite(_outputPins[outputNumber], (onOff)?HIGH:LOW );
	return this->GetState(outputNumber);
}
/*
	Returns the state (on / off) of a single output
*/
bool OutputsManager::GetState(uint8_t outputNumber)
{
	return this->_outputPinsState[outputNumber];
}
/*
	Simply cycles through the outputs switching them on / off then returns
	them to their original state.
*/
void OutputsManager::Test()
{
	for(int i=0;i<5;i++)
	{
		this->SetState( B00010101 );	
		delay( 150 );
		this->SetState( B00101010 );	
		delay( 150 );		
	}
	
	this->SetState( B00000000 );

	for(int i=0;i<5;i++)
	{
		// Knightrider!!11one
		this->SetState( B00000001 );delay( 50 );
		this->SetState( B00000011 );delay( 50 );
		this->SetState( B00000111 );delay( 50 );
		this->SetState( B00001110 );delay( 50 );
		this->SetState( B00011100 );delay( 50 );
		this->SetState( B00110000 );delay( 50 );
		this->SetState( B00100000 );delay( 50 );
		this->SetState( B00110000 );delay( 50 );
		this->SetState( B00111000 );delay( 50 );
		this->SetState( B00011100 );delay( 50 );
		this->SetState( B00001110 );delay( 50 );
		this->SetState( B00000111 );delay( 50 );
		this->SetState( B00000011 );delay( 50 );
		this->SetState( B00000001 );delay( 50 );
	}

	this->SetState( B00000000 );
}
/*
	Sets the fan speed based on the current temperature.  It tries to respect the 
	min / max fan speed settings, while it provides different speeds to the intake 
	and exhaust fans for negative feedback.  Here is an outline of the functions it performs.

		1.	Calculates a fan rate (duty cycle) based upon the current temperature by calling 
			the ControlFanSpeed() method of the EC_Temperature parent class. If the current 
			temperature is greater than max allowed temperature the duty cycle is set to 
			100% and enters cooling mode.  Then it sets the temperature that it needs to cool 
			too, by taking the hysteresis value away from the max temperature.

		2.	Once the call to ControlFanSpeed() has been made, the heating flag is checked and 
			the heater output is turned on or off accordingly.

			N.B.  Working in this way makes it possible to have the fans on (cooling mode) and 
			the heater on (heating mode) at the same time.  This allows for humidity control, 
			however care should be taken.

		3.	Checks to ensure that the calculated rate (duty cycle) is not outside of out max 
			/ min fan speed setting.  

		4.	Check to ensure that the input fan duty cycle is less then the exhaust fan duty
			cycle. The amount is determined by NegFBack variable as a percentage.

		5.	If the duty cycle or state of a fan has changed, the state (on/off) and duty cycle
			is updated.
*/
void OutputsManager::MaintainTemperature(float currentTemp, short tempMin, short tempMax, short hyst, float NegFBack, 
	short Fan1Min, short Fan1Max, short Fan2Mmin, short Fan2Max)
{
	fanStateHasChanged = false;
	ControlFanSpeed(currentTemp, tempMin, tempMax, hyst, NegFBack, Fan1Min, Fan1Max, Fan2Mmin, Fan2Max);

	// Turn heater on or off, but only if not already in that state.
	if( heating  != this->GetState( 3 ) )
	{
		SetState( 3, heating );
	}

	// Turn fans on or off, but only if not already in that state.
	if( (this->inFanSpeed > 0) != this->InFanState ) 
	{
		this->InFanState = this->SetState(4, (inFanSpeed > 0)?true:false );	
		fanStateHasChanged = true;
	}
	
	if( ((this->outFanSpeed > 0)?true:false) != this->outFanState ) 
	{
		this->outFanState = this->SetState(5, (outFanSpeed > 0)?true:false );
		fanStateHasChanged = true;
	}
	
	int dutyCycle = 0;
	
	for(int PWMOutput = 0; PWMOutput<2; PWMOutput++)
	{
		// Which output?
		switch(PWMOutput)
		{
			case INTAKE_FAN: 
				dutyCycle	= inFanSpeed;
				break;
			case EXHAUST_FAN:
				dutyCycle	= outFanSpeed;
				break;
		}

		// Detect if the new duty cycle is different from the one currently set.
		fanSpeedHasChanged	= (dutyCycle != _currentDutyCycle[PWMOutput]);
		
		// Set the duty cycle for the output, only if state or speed has changed
		if(fanSpeedHasChanged | fanStateHasChanged)
		{
			// Record the dutycycle for this output.
			_currentDutyCycle[PWMOutput] = dutyCycle;

			// Set the duty cycle for this output
			this->SetPWMOutputValue(PWMOutput, dutyCycle);
		}
	}
}
/*	
	Sets the duty cycle for a PWM output
*/
void OutputsManager::SetPWMOutputValue(int PWMOutputPin, short dutyCycle)
{
	// Ignore if out of bounds
	if(PWMOutputPin > 1 | PWMOutputPin < 0 )
		return;
	
	// Set PWM duty cycle for an output.
	analogWrite(_PWMOutputPins[PWMOutputPin], dutyCycle);
	delay(1);

	#if defined(DEBUG_FAN_CONTROLLER)
		Serial.print("Setting ");
		if( PWMOutputPin == 0 )
		{
			Serial.print("intake fan speed:");
			Serial.print(map(inFanSpeed,0,255,0,100));
			Serial.print("% (");Serial.print(inFanSpeed);
			Serial.println(")");
		}
		else
		{
			Serial.print("exhaust fan speed:");
			Serial.print(map(outFanSpeed,0,255,0,100));
			Serial.print("% (");Serial.print(outFanSpeed);
			Serial.println(")");
		}
	#endif
}