// EC_Interface.h

#ifndef _EC_INTERFACE_h
#define _EC_INTERFACE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <Arduino.h>
#include <LiquidCrystal.h>
#include "EC_Interface_Types.h"

class EC_Interface : LiquidCrystal
{
 private:
	boolean A_set;						// Stores state of encoders 'A' signal.
	boolean B_set;						// Stores state of encoders 'B' signal.
	long modeIncrementTimer;			// Timer used to calculate the how long the mode button was pressed.
	long ButtonDebounceTimer;			// Timer used to calculate bounce / press.
	long buttonPressTimer;				// The last time a button was pressed.
	int ButtonModeState;				// The current state of the mode button.
	int ButtonModeLastState;			// The previous state of the mode button.		
	void ModeIncrement();				// Increments the current MODE
	void ProcessModeButton();			// Handles mode button press from user.
	void SwitchInterfaceLED();			// Switches the interface LED dependant upon current MODE
	void UpdateSetupValues( volatile double encoderValue );
	int SmlPrt(double d);
	void begin(uint8_t,uint8_t);		// LicquidCrystal overrides.
	void DrawSplashScreen();			// Draws simple splash screen.

 public:
	volatile float encoderPos;			// A counter for the encoder.
	boolean rotating;					// Debounce management for the encoder.		
	ZC_MODES MODE;						// Enumeration used to indicate the zone controllers current mode.
	ZC_MODES LAST_MODE;					// Enumeration holds the zone controllers previous mode. 
	uint32_t SplashDelay;				// Delay in miliseconds when showing splash screen		
	EC_Interface(uint32_t SplashDelay);	// Constructor
	void SetupInterface();				// Initialises the LCD display
	void ProcessUserInterface();		// Handles interface input / output	
	void DrawInterface(bool clear=true);// Draws an interface on LCD dependant upon current MODE
	void UpdateHomeScreen(double tNow, double tMin,double tMax,
		double hNow,double hMin, double hMax,int lNow, 
		bool networkStatus );			// Updates homescreen
	void doEncoderA();					// Handles rotary encoder interrupt for channel A.
	void doEncoderB();					// Handles rotary encoder interrupt for channel B.	
	void setCursor(uint8_t,uint8_t);	// LicquidCrystal overrides.
	void print(String s);				// LicquidCrystal overrides.
	void print(int s);					// LicquidCrystal overrides.
	void print(uint8_t);				// LicquidCrystal overrides. prints in HEX
	void print(uint16_t);				// LicquidCrystal overrides. prints in HEX
	bool SettingsChanged;
};

#endif

