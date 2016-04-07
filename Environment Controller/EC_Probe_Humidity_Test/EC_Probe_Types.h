/******************************************************************************
	NAME   : EC_Probe_Types.h
	Created: 30/07/2012
	Author : Tom Odulate
	NOTES  : All pin & type definitions used by the EC Probe.

 ******************************************************************************/

#ifndef _EC_PROBE_TYPES_h
#define _EC_PROBE_TYPES_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

// Pin definitions
#define RS232_RX					0		// RS485 (INPUT)
#define RS232_TX					1		// RS485 (OUTPUT)
#define RS232_RTS					2		// RS485 (OUTPUT)
#define T_SENSOR_PIN				3		// Temperature Sensor  (INPUT)
#define H_SENSOR_PIN				A0		// Humidity Sensor  (INPUT)
#define L_SENSOR_PIN				5		// LDR sensor (INPUT)

#define PROBE_ADDRESS				1		// The modbus network address
#define PROBE_VERSION				1		// The version number of this firmware.
#define MAX_HUM_SAMPLES				10		// Total number of samples to take when reading humidity
#define MAX_TEMP_SAMPLES			5		// Total number of samples to take when reading temperature
#define SAMPLE_DELAY				10		// The delay time between taking humidity samples
#define MODBUS_ID					1		// Used to identify probe on the Modbus RTU network
#define MODBUS_BAUDRATE				4800	// Defines the comminication baud rate.
#define RH(time) ((.1667*time)-67)			// Relative Humidity calculation constant

#define GetFractionalPart(value) ((int)((value - (int)value)*100))
#define ToFractionalPart(value)  ((float)value / 100.00f )

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
  TOTAL_ERRORS,		// Used by modbus as lower level, leave this one here!
  TOTAL_REGS_SIZE	// Total number of registers for function 3 and 16 share the same register array
};

unsigned int holdingRegs[TOTAL_REGS_SIZE];	// function 3 and 16 register array

#endif

