/******************************************************************************
	NAME   : EC_Probe_Types.h
	Created: 30/07/2012
	Author : Tom Odulate
	NOTES  : All pin & type definitions used by the EC Probe.

			NB. It appears that the arduino bootloader for the ATTiny84 has
			the definitions for the I/O pins 8 & 10, the wrong way around!

 ******************************************************************************/

#ifndef _EC_PROBE_TYPES_h
#define _EC_PROBE_TYPES_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

// Pin definitions
#define RS232_RTS					8		// RS485 (INPUT, actually shown as INT10 on ATTiny84 datasheet!)
#define T_SENSOR_PIN				0		// Temperature Sensor  (INPUT)
#define H_SENSOR_PIN				1		// Humidity Sensor  (INPUT)
#define L_SENSOR_PIN				2		// LDR sensor (INPUT)

// Sensor readings
#define MAX_HUM_SAMPLES				20		// Total number of samples to take when reading humidity
#define MAX_TEMP_SAMPLES			20		// Total number of samples to take when reading temperature
#define SAMPLE_DELAY				5		// The delay time between taking sensor samples

// Helper macro
#define GetFractionalPart(value) ((int)((value - (int)value )*100 ))
#define ToFractionalPart(value)  ((float)value / 100.00f )

// Modbus network stuff
#define PROBE_ADDRESS				1		// The modbus network address
#define PROBE_VERSION				1		// The version number of this firmware.
#define MODBUS_BAUDRATE				28800L	// Defines the comminication baud rate.

/*  Enum used to reference the probe Registers, just add or remove registers as needed.
	The first register 	starts at address 0									
*/
enum 
{  
  TMP1,				// Temperatre value (left of decimal point)
  TMP2,				// Temperature fractional part value (right of decimal point)
  HUM1,				// Humidity value (left of decimal)
  HUM2,				// Humidity fractional part value (right of decimal point)
  LIGHT,			// Current light level from LDR.
  VERSION,			// Firmware version.
  TOTAL_ERRORS,		// Used by modbus at lower level, leave this one here!
  TOTAL_REGS_SIZE	// Total number of registers for function 3 and 16 share the same register array
};

unsigned int holdingRegs[TOTAL_REGS_SIZE];	// function 3 and 16 register array

#endif

