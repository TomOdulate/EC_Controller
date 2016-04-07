/****************************************************************************** 
	NAME   : EC_Interface_Types.h
	Version: 1
	Created: 27/06/2013
	Author : Thomas Odulate
	NOTES  : Firmware for the LCD display & rotary encoderfor the EC controller
			NB: LCD_RW is hardwired on the board.

******************************************************************************/
#include <Arduino.h>

#ifndef _EC_INTERFACE_TYPES_h
#define _EC_INTERFACE_TYPES_h

// Physical pin definitions.

#define LCD_RS		49	// LCD interface reset		(OUTPUT)
#define LCD_ENABLE	48	// LCD interface enable		(OUTPUT)
#define LCD_DB4		47	// LCD interface data line	(OUTPUT)
#define LCD_DB5		46	// LCD interface data line	(OUTPUT)
#define LCD_DB6		45	// LCD interface data line	(OUTPUT)
#define LCD_DB7		44	// LCD interface data line	(OUTPUT)
#define ITF_BUTTON	43	// Interface mode button	(INPUT)
#define ITF_ENC_A	42	// Interface encoder a		(INPUT)
#define ITF_STATUS	41	// Interface status LED.	(OUTPUT)
#define ITF_ENC_B	40	// Interface encoder b		(INPUT)


// Timings

#define DB_TIME			50L		// The button debounce time.
#define MODE_TIME		300L	// The time to wait before incrementing the mode.
#define MODE_RESET_TIME	5000L	// The time to wait for input before resetting the mode.


// Mode names & enumerations.
enum ZC_MODES { NORMAL, SET_TEMP_MAX, SET_TEMP_MIN,  SET_MAX_FAN1_SPEED, 
				SET_MIN_FAN1_SPEED, SET_FAN1_IDLE_SPEED, 
				SET_YEAR, SET_MONTH, SET_DAY, 
				SET_HOUR, SET_MIN, SET_SEC};

const String MODENAMES[12]	= {	"Normal\0",				// Friendly names for modes to display to user.
								"Set Max temp\0",
								"Set Min temp\0",
								"Set Max Fan1 Speed\0",
								"Set Min Fan1 Speed\0",
								"Set Fan1 idle Speed\0",
								"Set Year\0",
								"Set Month\0",
								"Set Day\0",
								"Set Hours\0",
								"Set Minutes\0",
								"Set Seconds\0"};
#endif

