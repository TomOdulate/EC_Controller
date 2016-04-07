/****************************************************************************** 
	NAME   : EC_Interface.h
	Created: 06/10/2012
	Author : Thomas Odulate
	NOTES  : Refer to header file.

 ******************************************************************************/

#include "EC_Interface.h"

EC_Interface::EC_Interface(uint32_t sDelay) : LiquidCrystal( LCD_RS, LCD_ENABLE, LCD_DB4, LCD_DB5, LCD_DB6, LCD_DB7 )
{
	encoderPos = 0;					// Reset counter for the encoder.
	//lastReportedPos = 1;			// Change management for the encoder.
	rotating = false;				// Reset rotation management for the encoder.
	A_set	= false;				// Reset state of encoders 'A' signal.
	B_set	= false;				// Reset state of encoders 'B' signal.	
	modeIncrementTimer	= 0;		// Reset timer used to calculate the how long the mode button was pressed.
	ButtonDebounceTimer	= 0;		// Reset timer used to calculate bounce / press.
	buttonPressTimer	= 0;		// Reset the last time a button was pressed.
	ButtonModeState		= HIGH;		// Reset the current state of the mode button.
	ButtonModeLastState	= HIGH;		// Reset the previous state of the mode button.	
	SettingsChanged		= false;	// Reset the flag that indicates if a save is required	
	SplashDelay			= sDelay;	// Set the amount of time to show the splash screen for.
}

void EC_Interface::begin(uint8_t a,uint8_t b)
{
	LiquidCrystal::begin(a,b);
}

void EC_Interface::SetupInterface()
{
	// Set our interface button / encoder & LED pin outputs / inputs.
	pinMode(ITF_BUTTON, INPUT); 
	pinMode(ITF_ENC_A, INPUT); 
	pinMode(ITF_ENC_B, INPUT); 
	pinMode(ITF_STATUS, OUTPUT); 

	// Set our LCD pin outputs.
	pinMode(LCD_DB4, OUTPUT); 
	pinMode(LCD_DB5, OUTPUT); 
	pinMode(LCD_DB6, OUTPUT); 
	pinMode(LCD_DB7, OUTPUT);
	
	// Turn on / off any pullup resistors
	digitalWrite(ITF_STATUS, LOW);
	digitalWrite(ITF_ENC_A, HIGH);
	digitalWrite(ITF_ENC_B, HIGH);
	digitalWrite(ITF_BUTTON, HIGH);

	// Initialise LCD display
	LiquidCrystal::begin(20, 4);
	
	// Create some custom characters & upload them to the display
	byte networkOk[8]	= { 0b00100, 0b00100, 0b01110, 0b00100, 0b00100, 0b01110, 0b00100, 0b00100 };
	byte networkError[8]= { 0b00100, 0b00100, 0b01110, 0b00000, 0b00000, 0b01110, 0b00100, 0b00100 };
	byte hBar[8]		= { 0b00000, 0b00000, 0b00000, 0b11111, 0b00000, 0b00000, 0b00000, 0b00000 };
	byte vBar[8]		= { 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100 };
	byte cnrTL[8]		= { 0b00000, 0b00000, 0b00000, 0b00111, 0b00100, 0b00100, 0b00100, 0b00100 };
	byte cnrBL[8]		= { 0b00100, 0b00100, 0b00100, 0b00111, 0b00000, 0b00000, 0b00000, 0b00000 };
	byte cnrBR[8]		= { 0b00100, 0b00100, 0b00100, 0b11100, 0b00000, 0b00000, 0b00000, 0b00000 };
	byte cnrTR[8]		= { 0b00000, 0b00000, 0b00000, 0b11100, 0b00100, 0b00100, 0b00100, 0b00100 };	
	LiquidCrystal::createChar(0, networkOk);
	LiquidCrystal::createChar(1, networkError);
	LiquidCrystal::createChar(2, cnrTL);  
	LiquidCrystal::createChar(3, vBar); 
	LiquidCrystal::createChar(4, cnrBL); 
	LiquidCrystal::createChar(5, hBar); 
	LiquidCrystal::createChar(6, cnrBR);
	LiquidCrystal::createChar(7, cnrTR);

	// Display the boot up screen.
	this->DrawSplashScreen();							
}

void EC_Interface::DrawSplashScreen()
{
	LiquidCrystal::setCursor(0,0);
	LiquidCrystal::
    LiquidCrystal::write(2);
    for(int i=1;i<19;i++) {
      LiquidCrystal::write(5);	  
	}
    LiquidCrystal::write(7);
    LiquidCrystal::setCursor(19,1);
    LiquidCrystal::write(3);
    LiquidCrystal::setCursor(19,2);
    LiquidCrystal::write(3);
    LiquidCrystal::setCursor(19,3);
    LiquidCrystal::write(3);
    LiquidCrystal::setCursor(0,1);
    LiquidCrystal::write(3);
    LiquidCrystal::setCursor(0,1);
    LiquidCrystal::write(3);
    LiquidCrystal::setCursor(0,1);
    LiquidCrystal::write(3);
    LiquidCrystal::setCursor(19,1);
    LiquidCrystal::write(3);    
    LiquidCrystal::setCursor(19,3);
    LiquidCrystal::write(6);
    LiquidCrystal::setCursor(18,3);
    LiquidCrystal::rightToLeft();
    for(int i=0;i<18;i++){      
      LiquidCrystal::write(5);
	}
    LiquidCrystal::write(4);
    LiquidCrystal::leftToRight();
    LiquidCrystal::setCursor(0,2);
    LiquidCrystal::write(3); 
	LiquidCrystal::setCursor(3,1);
	LiquidCrystal::print("E.C Controller\0");
	LiquidCrystal::setCursor(5,2);
	LiquidCrystal::print("version 0.1\0");
	delay(SplashDelay);
}

void EC_Interface::ProcessUserInterface()
{
	this->ProcessModeButton();					// Handle any mode button presses.
	
	if( MODE != LAST_MODE )						// Was there a chage of mode?
	{			
		LAST_MODE = MODE;						// Store the current mode, for the next time around.
		DrawInterface();						// Re-draw the display to display new mode.		
	}

	// Detect a Mode button timeout
	if( ( (millis() - buttonPressTimer) > MODE_RESET_TIME) & MODE != NORMAL )
	{		
		MODE = NORMAL;							// Reset the mode. I.e return to Home screen.
		DrawInterface();						// Re-draw the display to display home screen.		
	}		
}
	 
void EC_Interface::ProcessModeButton()
{
	int reading = digitalRead(ITF_BUTTON);						// Read state of the switch

	if (reading != ButtonModeLastState)							// If the switch changed, due to noise or a press
		ButtonDebounceTimer = millis();							// Reset the debouncing timer

	if ((millis() - ButtonDebounceTimer) > DB_TIME)	// If press was long enough...
		ButtonModeState = reading;								// Take this as a reading / press
  
	if(ButtonModeState == LOW)									// If the button was pressed
	{		
		buttonPressTimer = millis();							// Record 'when' mode button was pressed.
		ModeIncrement();										// Change the current mode
	}

	ButtonModeLastState = reading;								// Store the current reading for next iteration.
}

void EC_Interface::SwitchInterfaceLED()
{
	(MODE == NORMAL)?digitalWrite(ITF_STATUS,HIGH):digitalWrite(ITF_STATUS,LOW);
}

void EC_Interface::ModeIncrement()
{		
	switch( MODE )
	{
		case SET_TEMP_MAX:
			if( millis() - modeIncrementTimer > MODE_TIME )
			{
				//encoderPos = zone->GetMinTempThreshold();
				MODE = SET_TEMP_MIN;
				modeIncrementTimer = millis();
				DrawInterface();
			}
			break;		
		case SET_TEMP_MIN:			
			if( millis() - modeIncrementTimer > MODE_TIME )
			{
				//encoderPos = zone->GetMaxFan1Speed();
				MODE = SET_MAX_FAN1_SPEED;
				modeIncrementTimer = millis();
				DrawInterface();
			}
			break;		
		case SET_MAX_FAN1_SPEED:			
			if( millis() - modeIncrementTimer > MODE_TIME )
			{
				//encoderPos = zone->GetMinFan1Speed();
				MODE = SET_MIN_FAN1_SPEED;
				modeIncrementTimer = millis();
				DrawInterface();
			}
			break;
		case SET_MIN_FAN1_SPEED:
			if( millis() - modeIncrementTimer > MODE_TIME )
			{
				//encoderPos = zone->GetMinFan1Speed();
				MODE = SET_FAN1_IDLE_SPEED;
				modeIncrementTimer = millis();
				DrawInterface();
			}
			break;
		case SET_FAN1_IDLE_SPEED:
			if( millis() - modeIncrementTimer > MODE_TIME )
			{
				//encoderPos = zone->GetMaxTempThreshold();
				MODE = SET_YEAR;
				modeIncrementTimer = millis();
				DrawInterface();
			}
			break;
		case SET_YEAR:
			if( millis() - modeIncrementTimer > MODE_TIME )
			{
				//encoderPos = zone->GetMaxTempThreshold();				
				MODE = SET_MONTH;				
				modeIncrementTimer = millis();
				DrawInterface();
			}
			break;
		case SET_MONTH:
			if( millis() - modeIncrementTimer > MODE_TIME )
			{
				//encoderPos = zone->GetMaxTempThreshold();				
				MODE = SET_DAY;				
				modeIncrementTimer = millis();
				DrawInterface();
			}
			break;
		case SET_DAY:
			if( millis() - modeIncrementTimer > MODE_TIME )
			{
				//encoderPos = zone->GetMaxTempThreshold();				
				MODE = SET_HOUR;				
				modeIncrementTimer = millis();
				DrawInterface();
			}
			break;		
		case SET_HOUR:
			if( millis() - modeIncrementTimer > MODE_TIME )
			{
				//encoderPos = zone->GetMaxTempThreshold();				
				MODE = SET_MIN;				
				modeIncrementTimer = millis();
				DrawInterface();
			}
			break;
		case SET_MIN:
			if( millis() - modeIncrementTimer > MODE_TIME )
			{
				//encoderPos = zone->GetMaxTempThreshold();				
				MODE = SET_SEC;				
				modeIncrementTimer = millis();
				DrawInterface();
			}
			break;		
		case SET_SEC:
			if( millis() - modeIncrementTimer > MODE_TIME )
			{
				//encoderPos = zone->GetMaxTempThreshold();				
				MODE = SET_TEMP_MAX;				
				modeIncrementTimer = millis();
				DrawInterface();
			}
			break;		
		default:			
			if( millis() - modeIncrementTimer > MODE_TIME )
			{
				//encoderPos = zone->GetMaxTempThreshold();
				MODE = SET_TEMP_MAX;
				modeIncrementTimer = millis();
				DrawInterface();				
			}
	}	
	LAST_MODE = MODE;	
}

void EC_Interface::DrawInterface(bool clear)
{
	//double d = zone->GetMaxTempThreshold();
	char ss[4];
	if(clear) LiquidCrystal::clear();
	
	if( MODE != NORMAL)
	{
		LiquidCrystal::setCursor(5,0);
		LiquidCrystal::print("SETUP MODE");
		LiquidCrystal::setCursor(0,1);
	}
	
	switch( MODE )
	{
		case SET_TEMP_MAX:						
			LiquidCrystal::setCursor(4,2);
			LiquidCrystal::print(MODENAMES[MODE]);
			LiquidCrystal::setCursor( 8,3 );			
			//dtostrf(zone->GetMaxTempThreshold(), 4, 1, ss);
			LiquidCrystal::print( ss );
			break;		
		case SET_TEMP_MIN:		
			LiquidCrystal::setCursor(4,2);
			LiquidCrystal::print(MODENAMES[MODE]);
			LiquidCrystal::setCursor( 8,3 );
			//LiquidCrystal::print( zone->GetMinTempThreshold() );
			break;		
		case SET_MAX_FAN1_SPEED:
			LiquidCrystal::setCursor(1,2);
			LiquidCrystal::print(MODENAMES[MODE]);
			LiquidCrystal::setCursor( 8,3 );
			//LiquidCrystal::print( zone->GetMaxFan1Speed() );
			break;
		case SET_MIN_FAN1_SPEED:
			LiquidCrystal::setCursor(1,2);
			LiquidCrystal::print(MODENAMES[MODE]);
			LiquidCrystal::setCursor( 8,3 );
			//LiquidCrystal::print( zone->GetMinFan1Speed() );
			break;
		case SET_FAN1_IDLE_SPEED:
			LiquidCrystal::setCursor(0,2);
			LiquidCrystal::print(MODENAMES[MODE]);
			LiquidCrystal::setCursor( 8,3 );
			//LiquidCrystal::print( zone->GetFan1IdleSpeed() );
			break;
		case SET_YEAR:
			LiquidCrystal::setCursor(6,2);
			LiquidCrystal::print(MODENAMES[MODE]);
			LiquidCrystal::setCursor( 8,3 );
			LiquidCrystal::print( "2000" );
			break;
		case SET_MONTH:
			LiquidCrystal::setCursor(5,2);
			LiquidCrystal::print(MODENAMES[7]);
			LiquidCrystal::setCursor( 8,3 );
			LiquidCrystal::print( "01" );
			break;
		case SET_DAY:
			LiquidCrystal::setCursor(5,2);
			LiquidCrystal::print(MODENAMES[8]);
			LiquidCrystal::setCursor( 8,3 );
			LiquidCrystal::print( "1st" );
			break;
		default:   
			// Home screen.
			if(clear) LiquidCrystal::clear();
			LiquidCrystal::setCursor(0,0);
			//LiquidCrystal::print(zone->GetName());
			
			// Header Line
			LiquidCrystal::setCursor(5,0);  LiquidCrystal::print("Now");
			LiquidCrystal::setCursor(10,0); LiquidCrystal::print("Min");
			LiquidCrystal::setCursor(15,0); LiquidCrystal::print("Max");	
			
			// Temperature line
			LiquidCrystal::setCursor(0, 1);LiquidCrystal::print("Tmp:");		
			
			// Humidity Line
			LiquidCrystal::setCursor(0, 2);LiquidCrystal::print("Hum:");

			// Light Level line
			LiquidCrystal::setCursor(0, 3);LiquidCrystal::print("Lig:");
			break;
	}	
	
	SwitchInterfaceLED();	// Set the interface LED accordingly.
}

void EC_Interface::UpdateSetupValues( volatile double encoderValue )
{
	if(MODE == NORMAL) return;			// Only interested in setup modes!
	LiquidCrystal::setCursor( 8,3 );	// Set the correct cursor position.
	SettingsChanged = true;
	switch( MODE )						// If you need to adjust the value, e.g from float to %, do it here (perhaps!).
	{
		/*case SET_TEMP_MAX: data.TempMax = encoderValue; break;		
		case SET_TEMP_MIN: data.TempMin = encoderValue; break;				
		case SET_MAX_FAN1_SPEED:data.Fan1MaxSpeed = encoderValue; break;
		case SET_MIN_FAN1_SPEED:data.Fan1MinSpeed = encoderValue;break;
		case SET_FAN1_IDLE_SPEED:data.Fan2MinSpeed = encoderValue;break;*/
		
		//case SET_TEMP_MAX: zone->SetMaxTemp(encoderValue); break;		
		//case SET_TEMP_MIN: zone->SetMinTemp(encoderValue); break;				
		//case SET_MAX_FAN1_SPEED:zone->SetMaxFan1Speed( encoderValue ); break;
		//case SET_MIN_FAN1_SPEED:zone->SetMinFan1Speed( encoderValue );break;
		//case SET_FAN1_IDLE_SPEED:zone->SetFan1IdleSpeed( encoderValue );break;
	}

	LiquidCrystal::print( encoderValue );		// Update the value;
}

void EC_Interface::UpdateHomeScreen(double tNow,double tMin,double tMax,double hNow,double hMin,double hMax, int lNow, bool networkError )
{	
	LiquidCrystal::setCursor(0,0);

	if( networkError )
	{
		LiquidCrystal::write((uint8_t)1);
	}
	else 
		LiquidCrystal::write((uint8_t)0);
	
	char buffer[4];
	LiquidCrystal::setCursor(5,1);	
	sprintf(buffer,"%2d.%d", (int)constrain(tNow,0,99.9), SmlPrt(tNow));
	LiquidCrystal::print(buffer);

	LiquidCrystal::setCursor(10,1);
	sprintf(buffer,"%2d.%d", (int)constrain(tMin,0,99.9), SmlPrt(tMin));
	LiquidCrystal::print(buffer);
		
	LiquidCrystal::setCursor(15,1);
	sprintf(buffer,"%2d.%d", (int)constrain(tMax,0,99.9), SmlPrt(tMax));
	LiquidCrystal::print(buffer);
	
	LiquidCrystal::setCursor(5,2);
	sprintf(buffer,"%2d.%d", (int)constrain(hNow,0,99.9), SmlPrt(hNow));
	LiquidCrystal::print(buffer);
	
	LiquidCrystal::setCursor(10,2);
	sprintf(buffer,"%2d.%d", (int)constrain(hMin,0,99.9), SmlPrt(hMin));
	LiquidCrystal::print(buffer);
	
	LiquidCrystal::setCursor(15,2);
	sprintf(buffer,"%2d.%d", (int)constrain(hMax,0,99.9), SmlPrt(hMax));
	LiquidCrystal::print(buffer);

	LiquidCrystal::setCursor(5,3);
	sprintf(buffer,"%4d", lNow);
	LiquidCrystal::print(buffer);
}

void EC_Interface::doEncoderA()
{
	if( MODE != NORMAL )
	{
		// debounce
		if ( rotating ) 
			delay (1);  // wait a little until the bouncing is done

		// Test transition, did things really change? 
		if( digitalRead(ITF_ENC_A) != A_set ) 
		{											// debounce once more
			A_set = !A_set;

			// Adjust counter + if A leads B
			if ( A_set && !B_set ) 
			{
				buttonPressTimer = millis();							// Reset the button pressed timer
				if(encoderPos < 100.00) 
					encoderPos += 0.50;

				UpdateSetupValues(encoderPos);   
			}

		rotating = false;  // no more debouncing until loop() hits again
	}	
  }
}

void EC_Interface::doEncoderB()
{
	if( MODE != NORMAL )
	{
		if ( rotating ) 
			delay (1);

		if( digitalRead(ITF_ENC_B) != B_set ) 
		{
			B_set = !B_set;
			//  adjust counter - 1 if B leads A
			if( B_set && !A_set ) 
			{
				buttonPressTimer = millis();							// Reset the button pressed timer

				if(encoderPos > 0)
					encoderPos -= 0.50;
							
				UpdateSetupValues(encoderPos);	
			}

			rotating = false;
		}
	}
}

void EC_Interface::setCursor(uint8_t cols,uint8_t rows)
{
	LiquidCrystal::setCursor(cols,rows);
}

void EC_Interface::print(String s)
{
	LiquidCrystal::print(s);
}

void EC_Interface::print(int s)
{
	LiquidCrystal::print(s);
}

void EC_Interface::print(uint8_t s)
{
	LiquidCrystal::print(s,HEX);
}

void EC_Interface::print(uint16_t s)
{
	LiquidCrystal::print(s,HEX);
}

int EC_Interface::SmlPrt(double d)
{	
	return ((d - (int)d) * 10);
}

