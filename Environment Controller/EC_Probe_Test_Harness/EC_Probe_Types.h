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

#define EC_PROBE_VERSION			0.10	// The version number of this firmware.
#define EC_PROBE_BAUDRATE			4800	// Defines the comminication baud rate.
#define RH(time) ((.1667*time)-67)			// Relative Humidity calculation constant
#define MAX_HUM_SAMPLES				10L		// Total number of samples to take when reading humidity
#define MAX_TEMP_SAMPLES			5L		// Total number of samples to take when reading temperature
#define SAMPLE_DELAY				10L		// The delay time between taking humidity samples

// Pin definitions
#define T_SENSOR_PIN				A0		// Temperature Sensor  (INPUT)
#define H_SENSOR_PIN				A1		// Humidity Sensor  (INPUT)
#define RS232_TX					1		// RS485 (OUTPUT)
#define RS232_RX					0		// RS485 (INPUT)
#define RS232_RTS					2		// RS485 (OUTPUT)



#endif

