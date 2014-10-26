/*
 The MySensors library adds a new layer on top of the RF24 library.
 It handles radio network routing, relaying and ids.

 Created by Henrik Ekblad <henrik.ekblad@gmail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

#ifndef MyMessage_h
#define MyMessage_h

#ifdef __cplusplus
#include <Arduino.h>
#include <string.h>
#include <stdint.h>
#endif

#define PROTOCOL_VERSION 3
#define MAX_MESSAGE_LENGTH 32
#define HEADER_SIZE 7									// SETt/REQ command header size
#define MAX_PAYLOAD (MAX_MESSAGE_LENGTH - HEADER_SIZE)	// Max payload for SET/REQ commands

/// Message types
typedef enum {
	/// Internal commands used by the API
	C_INTERNAL = 0,
	/// Push data or comands to sensor or controller
	C_SET = 1,
	/// Request data from sensor or controller
	C_REQ = 2,
	/// Firmware OTA transmissions.
	C_FIRMWARE = 3
} MySensorCommand;

/// Sensor types used for I_PRESENTATION messages.
/// The sensors always report in SI units. See table below for details.
typedef enum {
	/// Node itself, used for internal commands and meta data.
	/// See I_xxx value types below.
	S_NODE,

	/// Door sensor
	/// V_ARMED - 1 = Armed, 0 = Bypassed
	/// V_STATUS - 1 = Tripped (open), 0 = Untripped (closed=
	S_DOOR,

	/// Window sensor
	/// V_ARMED - 1 = Armed, 0 = Bypassed
	/// V_STATUS - 1 = Tripped (open), 0 = Untripped (closed)
	S_WINDOW,

	/// Motion sensor
	/// V_ARMED - 1 = Armed, 0 = Bypassed
	/// V_STATUS - 1 = Tripped (motion detected), 0 = Untripped
	S_MOTION,

	/// Smoke sensor
	/// V_ARMED - 1 = Armed, 0 = Bypassed
	/// V_STATUS - 1 = Tripped (fire), 0 = Untripped
	S_SMOKE,

	/// Light (or other) on/off actuator
	/// V_STATUS - 1 - turn on, 0 = turn off
	/// V_WATT - Current power consumption in Watt <int>
	/// V_WATT_MAX - Max watt value
	/// V_WATT_MIN - Min watt value
	/// V_WATT_AVERAGE - Min watt value
	/// V_ACCUMULATED - The accumulated number of kWh <int or float>
	S_BINARY,

	/// Dimmable actuator
	/// V_STATUS - 1 - turn on, 0 = turn off
	/// V_PERCENTAGE - Dimmer level 0-100
	/// V_WATT - Current power consumption in Watt <int>
	/// V_WATT_MAX - Max watt value
	/// V_WATT_MIN - Min watt value
	/// V_WATT_AVERAGE - Min watt value
	/// V_ACCUMULATED - The accumulated number of kWh <int or float>
	S_DIMMABLE,

	/// RGB Light (with red, green, blue component)
	/// V_STATUS - 1 - turn on all components, 0 = turn off all components
	/// V_R_LEVEL - Red component 0-255 <int>
	/// V_G_LEVEL - Green component 0-255 <int>
	/// V_B_LEVEL - Blue component 0-255 <int>
	/// V_WATT - Watt <int>
	/// V_WATT_MAX - Max watt value
	/// V_WATT_MIN - Min watt value
	/// V_WATT_AVERAGE - Min watt value
	/// V_ACCUMULATED - The accumulated number of kWh <int or float>
	S_RGB,

	/// RGBW Light (with red, green, blue white component)
	/// V_STATUS - 1 - turn on all components, 0 = turn off all components
	/// V_R_LEVEL - Red component 0-255 <int>
	/// V_G_LEVEL - Green component 0-255 <int>
	/// V_B_LEVEL - Blue component 0-255 <int>
	/// V_W_LEVEL - White component 0-255 <int>
	/// V_WATT - Watt <int>
	/// V_WATT_MAX - Max watt value
	/// V_WATT_MIN - Min watt value
	/// V_WATT_AVERAGE - Min watt value
	/// V_ACCUMULATED - The accumulated number of kWh <int or float>
	S_RGBW,

	/// Window covers or shades
	/// V_STATUS - 1 - open (full up), 0 = close (full down)
	/// V_PERCENTAGE - Open cover 0-100% <int>
	/// V_STOP - stop blinds in the middle of motion.
	S_WINDOW_COVER,

	/// Temperature sensor
	/// V_LEVEL - Current temperature level in degrees celsius <int or float>
	/// V_LEVEL_MAX - Max temperature level
	/// V_LEVEL_MIN - Min temperature level
	/// V_LEVEL_AVERAGE - Average temperature level
	S_THERMOMETER,

	/// Humidity sensor
	/// V_PERCENTAGE - Humidity percentage 0-100% <int or float>
	/// V_PERCENTAGE_MAX - Max humidity percentage
	/// V_PERCENTAGE_MIN - Min humidity percentage
	/// V_PERCENTAGE_AVERAGE - Average humidity percentage
	S_HUMIDITY,

	/// Barometer sensor (Pressure)
	/// V_LEVEL - Pressure level in hPa
	/// V_MODE - Whether forecast. One of 0="stable", 1="sunny", 2="cloudy", 3="unstable", 4="thunderstorm" or 5="unknown"
	/// V_LEVEL_MAX - Max pressure level
	/// V_LEVEL_MIN - Min pressure level
	/// V_LEVEL_AVERAGE - Average pressure level
	S_BAROMETER,

	/// Wind sensor
	/// V_LEVEL - Wind level in m/s (average wind speed during last report period)
	/// V_LEVEL_MIN - Min wind level
	/// V_LEVEL_MAX - Max wind level (gust)
	/// V_LEVEL_AVERAGE - Average wind level-
	/// V_ANGLE - degrees clockwise from true north <int>
	S_WIND,

	/// Rain sensor
	/// V_STATUS - Current rain status. 1=raining, 0=not raining.
	/// V_ACCUMULATED - Accumulated rain in mm
	/// V_RATE - Rain rate in mm/h
	/// V_RATE_MAX - Max rain rate
	/// V_RATE_MIN - Min rain rate
	/// V_RATE_AVERAGE - Average rain rate
	S_RAIN,

	/// UV sensor
	/// V_LEVEL - Uv Index level (0-12)
	/// V_LEVEL_MAX - Max uv level
	/// V_LEVEL_MIN - Min uv level
	/// V_LEVEL_AVERAGE - Average uv level
	S_UV,

	/// Weight sensor
	/// V_LEVEL - Weight in kg <int, float>
	/// V_LEVEL_MAX - Max weight level
	/// V_LEVEL_MIN - Min weight level
	/// V_LEVEL_AVERAGE - Average weight level
	S_WEIGHT_SCALE,

	/// Power measuring device, like power meters or clamps
	/// V_WATT - Watt <int>
	/// V_WATT_MAX - Max watt value
	/// V_WATT_MIN - Min watt value
	/// V_WATT_AVERAGE - Min watt value
	/// V_ACCUMULATED - The accumulated number of kWh <int or float>
	S_POWER,

	/// Thermostat (for controlling heater or cooler devices)
	/// V_STATUS - 1=On, 0=Off. Heater/cooler power switch.
	/// V_MODE - Heater/AC mode. One of 0="Off", 1="HeatOn", 2="CoolOn", or 3="AutoChangeOver"
	/// V_LEVEL - Setpoint for ideal temperature in celsius degrees
	S_THERMOSTAT,

	/// Distance sensor
	/// V_LEVEL - Distance in meters <int, float>
	/// V_LEVEL_MAX - Max distance value
	/// V_LEVEL_MIN - Min distance value
	/// V_LEVEL_AVERAGE - Average distance value
	S_DISTANCE,

	/// Light sensor
	/// V_STATUS - Tripped status. Set to 1 when light is detected (binary light sensors).
	/// V_LEVEL - Light level in lux
	/// V_LEVEL_MAX - Max lux light level
	/// V_LEVEL_MIN - Min lux light level
	/// V_LEVEL_AVERAGE - Average lux light level
	S_LIGHT_SENSOR,

	/// Uncalibrated Light sensor
	/// V_PERCENTAGE - Light level in percentage 0-100% <int, float>
	/// V_PERCENTAGE_MAX - Max light level %
	/// V_PERCENTAGE_MIN - Min light level %
	/// V_PERCENTAGE_AVERAGE - Average light level %
	S_UNCALIBRATED_LIGHT_SENSOR,

	/// Binary light sensor which triggers at a certain level.
	/// V_ARMED - 1 = Armed, 0 = Bypassed
	/// V_STATUS - 1 = Tripped, 0 = Untripped
	S_BINARY_LIGHT_SENSOR,

	/// Lock device
	/// V_STATUS - 1=Locked, 0 = Unlocked
	S_LOCK,

	/// Ir sender or receiver device
	/// V_IR_RECEIVED - Ir code received
	/// V_IR_SEND - Ir code to send
	S_IR,

	/// Water meter
	/// V_ACCUMULATED - Accumulated water volume in m3 <int, float>
	/// V_RATE - Flow rate in l/m <int or float>
	/// V_RATE_MAX - Max flow rate
	/// V_RATE_MIN - Min flow rate
	/// V_RATE_AVERAGE - Average flow rate
	S_WATER_METER,

	/// Ph sensor
	/// V_LEVEL - Ph level using standard pH scale 0-14 <int or float>
	/// V_LEVEL_MAX - Max ph level
	/// V_LEVEL_MIN - Min ph level
	/// V_LEVEL_AVERGE - Average ph level
	S_PH,

	/// Scene controller device
	/// V_SCENE_ON - Scene number <int>
	/// V_SCENE_OFF - Scene number <int>
	S_SCENE_CONTROLLER,

	// Sound sensor
	/// V_LEVEL - Calibrated sound level in db
	/// V_LEVEL_MAX - Max sound level
	/// V_LEVEL_MIN - Min sound level
	/// V_LEVEL_AVERAGE - Average sound level
	S_SOUND,

	// Uncalibrated sound sensor
	/// V_PERCENTAGE - Uncalibrated sound level in percentage 0-100% <int, float>
	/// V_PERCENTAGE_MAX - Max sound level %
	/// V_PERCENTAGE_MIN - Min sound level %
	/// V_PERCENTAGE_AVERAGE - Average sound level %
	S_UNCALIBRATED_SOUND,

	// Sound sensor which triggers at a certain sound level
	/// V_STATUS - Tripped status. Set to 1 when sound is detected.
	/// V_ARMED - Armed status of a security sensor. 1=Armed, 0=Bypassed
	S_BINARY_SOUND,

	/// Vibration sensor
	/// V_LEVEL - vibration level in Hertz
	/// V_LEVEL_MAX - Max vibration level
	/// V_LEVEL_MIN - Min vibration level
	/// V_LEVEL_AVERAGE - Average vibration level
	S_VIBRATION,

	/// Vibration sensor which triggers at a certain level
	/// V_STATUS - Tripped status. Set to 1 when vibration is detected.
	/// V_ARMED - Armed status of a security sensor. 1=Armed, 0=Bypassed
	S_BINARY_VIBRATION,

	/// Gyro sensor
	/// Here we need some kind of value types. Help needed!
	S_GYRO,

	/// Compass sensor
	/// V_ANGLE - degrees clockwise from true north <int>
	S_COMPASS,

	// Leave some space in numbering here for future sensors

	/// Some more obscure gas sensors
	/// V_LEVEL - Gas level in ug/m3
	/// V_LEVEL_MAX - Max gas level
	/// V_LEVEL_MIN - Min gas level
	/// V_LEVEL_AVERAGE - Average gas level
	S_DUST=80,			// Dust sensor
	S_CARBON_MONOXIDE, 	// Carbon Monoxide – CO
	S_CARBON_DIOXIDE, 	// Carbon Dioxide – CO2
	S_OXYGENE, 			// Oxygen – O2
	S_METHANE,			// Methane – CH4
	S_HYDROGEN,			// Hydrogen – H2
	S_AMMONIA,			// Ammonia – NH3
	S_ISOBUTANE, 		// Isobutane – C4H10
	S_ETHANOL,			// Ethanol – CH3CH2OH (aka C2H5OH or C2H60)
	S_ETHANOL_50,		// Ethanol CO2H50H
	S_TOULENE,			// Toluene – C6H5CH3
	S_HYDROGEN_SULFIDE, // Hydrogen Sulfide – H2S
	S_NITROGEN_DIOXIDE, // Nitrogen Dioxide – NO2
	S_OZONE,			// Ozone – O3
	S_HYDROCARBON,		// Hydrocarbons – VOC
	S_CHLORINE,			// Chlorine CL2
	S_AMMONIUM,			// Ammonium NH4
	S_METHYL,			// Methyl CH3
	S_ACETONE, 			// Acetone CH3_2CO
	S_LPG,				// LPG (both C3H8 C4H10)
	S_NITRIC_OXIDE,		// Nitric oxide NO
	S_NITROGENE_OXIDES,	// Nitrogen oxides NOX
	S_NO3,				// NO3 ion (nitrate, not adjusted for ammonium ion) UG/M3
	S_S04,				// SO4 ion, sulfate, not adjusted for ammonium ion, UG/M3
	S_S02,				// SO2 sulfur dioxide, ppb
	S_EC,				// EC (elemental carbon) – PM2.5 UG/M3
	S_ORGANIC_CARBON,	// OC (organic carbon, not adjusted for oxygen and hydrogen) – PM2.5 UG/M3
	S_BLACK_CARBON, 	// BC (black carbon at 880 nm) UG/M3
	S_UV_AETH,			// UV-AETH (second channel of Aethalometer at 370 nm) UG/M3
	S_PM2,				// PM2.5 mass - UG/M3
	S_PM10,				// PM10 mass - PM10 mass

	/// Used for sensors not fitting any other sensor types.
	/// If you find yourself using this it might be a candidate for a new sensor type?
	S_CUSTOM=255,

} MySensorType;


/// Value types used internally by the API. INTERNAL cmd.
typedef enum {
	/// Used to report sketch or gateway version to controller. sendSketchInfo()
	I_VERSION,
	/// Sent from gateway to controller when it's ready for action.
	I_GATEWAY_READY,
	/// Present sensors attached to a node. Payload is the ChildSensorType.
	I_PRESENTATION,
	/// Request a new id from controller.
	I_ID,
	/// Broadcased message from a node to request neighbouring repeaters and gateway to
	/// report their distance to controller back.
	I_FIND_PARENT,
	/// Send in a log message to controllers application log.
	I_LOG_MESSAGE,

	// Report battery level. sendBatteryLevel()
	I_BATTERY_LEVEL,
	/// Request time from controller. requestTime()
	I_TIME,

	/// Reebot node command. Requires special bootloader on sensor node.
	I_RESET,
	/// Activate/deactivate inclusion mode. Used between gateway and controller.
	I_INCLUSION_MODE,
	/// Used to report sketch name to controller. sendSketchInfo()
	I_NAME,
} MySensorInternalValueType;

/// ValueTypes used for SET/REQ commands
typedef enum {

	// Leave some room for future internal value types

	/// A few custom config values available for feching configuration from another sensor
	/// or controller. Sender and receiver must have a common understanding on what to expect.
	V_CONFIG1=30,
	V_CONFIG3,
	V_CONFIG4,
	V_CONFIG5,

	/// A few custom variables or sensor data normally used to push information
	/// to another sensor or controller. Sender and receiver must have a common
	/// understanding on what to expect.
	V_VAR1,
	V_VAR2,
	V_VAR3,
	V_VAR4,
	V_VAR5,

	/// Use this for transmitting custom _binary_ data not covered by above.
	/// Requires sender/receiver to have a common understanding on the payload content.
	V_CUSTOM,

	/// Use STATUS for reporting or settings binary values such as a true/false on on/off state.
	/// 1 - turn on/tripped,... , 0 = turn off/not tripped,...
	/// See the MySensorType for usage on differend sensor types
	V_STATUS,

	/// ValueType mainly used by security devices. 1=Armed, 0=Disarmed.
	V_ARMED,

	/// Sensor or actuator level. E.g weight, light level,
	V_LEVEL,
	/// Used to report maximum level value in the node.
	V_LEVEL_MAX,
	/// Used to report minimum level value in the node.
	V_LEVEL_MIN,
	/// Used to report average level value in the node.
	V_LEVEL_AVERAGE,

	/// Report or set percentage value. E.g. dimmer level, window cover, humidity, uncalibrated light level.
	V_PERCENTAGE,
	/// Used to report maximum percentage from the node.
	V_PERCENTAGE_MAX,
	/// Used to report mainimum percentage from the node.
	V_PERCENTAGE_MIN,
	/// Used to report average percentage from the node.
	V_PERCENTAGE_AVERAGE,

	/// Used for accumelated values such as rain(mm) or KWh
	V_ACCUMULATED,
	/// Reset accumelated sensor value
	V_ACCUMULATED_RESET,

	/// Used to report rate value such as rain or water flow rate
	V_RATE,
	/// Used to report maximum rate value.
	V_RATE_MAX,
	/// Used to report minimum rate value
	V_RATE_MIN,
	/// Used to report average rate value
	V_RATE_AVERAGE,

	/// Used to report current power consuption in Watt
	V_WATT,
	/// Used to report maximum watt value.
	V_WATT_MAX,
	/// Used to report minimum watt value.
	V_WATT_MIN,
	/// Used to report average watt value.
	V_WATT_AVERAGE,

	/// Used for sensor or actuaor mode. E.g. heater, weather forecast prognosis
	V_MODE,

	/// Stop command to shut down motion of blinds or windows coverings.
	V_STOP,

	/// Red component 0-255 for led lightning
	V_R_LEVEL,
	/// Green component 0-255 for led lightning
	V_G_LEVEL,
	/// Blue component 0-255 for led lightning
	V_B_LEVEL,
	// White component 0-255 for led lightning
	V_W_LEVEL,

	/// Ir code received
	V_IR_RECEIVED,
	/// Ir code to send
	V_IR_SEND,

	/// Scene on command.
	V_SCENE_ON,
	/// Schene off command
	V_SCENE_OFF,

	/// Used for compasses or wind direction. Degrees clockwise from true north
	V_ANGLE

} MySensorVarValueType;


typedef enum {
	P_STRING, P_BYTE, P_INT16, P_UINT16, P_LONG32, P_ULONG32, P_CUSTOM, P_FLOAT32
} MySensorPayloadDataType;


/*
#define BIT(n)                  ( 1<<(n) )
// Create a bitmask of length len.
#define BIT_MASK(len)           ( BIT(len)-1 )
// Create a bitfield mask of length starting at bit 'start'.
#define BF_MASK(start, len)     ( BIT_MASK(len)<<(start) )

// Prepare a bitmask for insertion or combining.
#define BF_PREP(x, start, len)  ( ((x)&BIT_MASK(len)) << (start) )
// Extract a bitfield of length len starting at bit 'start' from y.
#define BF_GET(y, start, len)   ( ((y)>>(start)) & BIT_MASK(len) )
// Insert a new bitfield value x into y.
#define BF_SET(y, x, start, len)    ( y= ((y) &~ BF_MASK(start, len)) | BF_PREP(x, start, len) )

// Getters/setters for special bit fields in header
#define mSetVersion(_msg,_version) BF_SET(_msg.version_length, _version, 0, 3)
#define mGetVersion(_msg) BF_GET(_msg.version_length, 0, 3)

#define mSetLength(_msg,_length) BF_SET(_msg.version_length, _length, 3, 5)
#define mGetLength(_msg) BF_GET(_msg.version_length, 3, 5)

#define mSetCommand(_msg,_command) BF_SET(_msg.command_ack_payload, _command, 0, 3)
#define mGetCommand(_msg) BF_GET(_msg.command_ack_payload, 0, 3)

#define mSetRequestAck(_msg,_rack) BF_SET(_msg.command_ack_payload, _rack, 3, 1)
#define mGetRequestAck(_msg) BF_GET(_msg.command_ack_payload, 3, 1)

#define mSetAck(_msg,_ackMsg) BF_SET(_msg.command_ack_payload, _ackMsg, 4, 1)
#define mGetAck(_msg) BF_GET(_msg.command_ack_payload, 4, 1)

#define mSetPayloadType(_msg, _pt) BF_SET(_msg.command_ack_payload, _pt, 5, 3)
#define mGetPayloadType(_msg) BF_GET(_msg.command_ack_payload, 5, 3)


// internal access for special fields
#define miGetCommand() BF_GET(command_ack_payload, 0, 3)

#define miSetLength(_length) BF_SET(version_length, _length, 3, 5)
#define miGetLength() BF_GET(version_length, 3, 5)

#define miSetRequestAck(_rack) BF_SET(command_ack_payload, _rack, 3, 1)
#define miGetRequestAck() BF_GET(command_ack_payload, 3, 1)

#define miSetAck(_ack) BF_SET(command_ack_payload, _ack, 4, 1)
#define miGetAck() BF_GET(command_ack_payload, 4, 1)

#define miSetPayloadType(_pt) BF_SET(command_ack_payload, _pt, 5, 3)
#define miGetPayloadType() BF_GET(command_ack_payload, 5, 3)

*/

/**
 * Network header structures
 */

typedef struct
{
	/// 8 bit - Id of last node this message passed
	uint8_t last;

	/// 8 bit - Id of sender node (origin)
	uint8_t sender;

	// 8 bit - Id of destination node
	uint8_t destination;

	/// 1 bit - Request an ack - Indicator that receiver should send an ack back.
	/// 1 bit - Is ack messsage - Indicator that this is the actual ack message.
	/// 5 bit - Length of payload
	/// 1 bit - Reserved for future use
	uint8_t ack_length;

	/// 8 bit - Command type
	uint8_t command;
} NetworkHeaderDefault;


/**
 * Firmware command structures
 */

#define FIRMWARE_BLOCK_SIZE	16

typedef enum {
	ST_FIRMWARE_CONFIG_REQUEST, ST_FIRMWARE_CONFIG_RESPONSE, ST_FIRMWARE_REQUEST, ST_FIRMWARE_RESPONSE
} MySensorFirmwareCommandType;


typedef struct
{
} FirmwareConfigRequest;

typedef struct
{
	uint16_t blocks;
	uint16_t crc;
} FirmwareConfigResponse;

typedef struct
{
	uint16_t block;
} FirmwareRequest;

typedef struct
{
	uint16_t block;
	uint8_t data[FIRMWARE_BLOCK_SIZE];
} FirmwareResponse;

typedef struct {
	uint8_t type;
	uint8_t version;

	union {
		FirmwareConfigRequest configRequest;
		FirmwareConfigResponse configResponse;
		FirmwareRequest request;
		FirmwareResponse response;
    }  __attribute__((packed));;
} CmdFirmware;



/**
 * Req/set command structure
 */
typedef struct {
	/// 3 bit - Payload data type (MySensorPayloadDataType)
	/// 5 bit - Reserved
	uint8_t ptype;

	/// 8 bit - Id of sensor that this message concerns.
	uint8_t sensor;

	/// 8-bit. MySensorVarValueType. See above.
	uint8_t type;

	/// Each message can transfer a payload. We add one extra byte for string
	/// terminator \0 to be "printable" this is not transferred OTA
	/// This union is used to simplify the construction of the binary data types transferred.
	union {
		uint8_t bValue;
		unsigned long ulValue;
		long lValue;
		unsigned int uiValue;
		int iValue;
		struct { // Float messages
			float fValue;
			uint8_t fPrecision;   // Number of decimals when serializing
		};
		char data[MAX_PAYLOAD + 1];
	} __attribute__((packed));
} CmdVar;


/**
 * Internal command structure
 */
typedef struct {
	/// 8 bit - MySensorInternalValueType. See above.
	uint8_t type;

	/// Internal payload
	union {
		uint8_t bValue;
		unsigned long ulValue;
		long lValue;
		unsigned int uiValue;
		int iValue;
		char data[MAX_PAYLOAD + 1];
	} __attribute__((packed));
} CmdInternal;




#ifdef __cplusplus
class MyMessage
{
public:
	// Constructors
	MyMessage();

	MyMessage(uint8_t sensor, uint8_t type);

	char i2h(uint8_t i) const;

	/**
	 * If payload is something else than P_STRING you can have the payload value converted
	 * into string representation by supplying a buffer with the minimum size of
	 * 2*MAX_PAYLOAD+1. This is to be able to fit hex-conversion of a full binary payload.
	 */
	char* getStream(char *buffer) const;
	char* getString(char *buffer) const;
	const char* getString() const;
	void* getCustom() const;
	uint8_t getByte() const;
	bool getBool() const;
	float getFloat() const;
	long getLong() const;
	unsigned long getULong() const;
	int getInt() const;
	unsigned int getUInt() const;

	// Getter for ack-flag. True if this is an ack message.
	bool isAck() const;

	// Setters for building message "on the fly"
	MyMessage& setType(uint8_t type);
	MyMessage& setSensor(uint8_t sensor);
	MyMessage& setDestination(uint8_t destination);

	// Setters for payload
	MyMessage& set(void* payload, uint8_t length);
	MyMessage& set(const char* value);
	MyMessage& set(uint8_t value);
	MyMessage& set(float value, uint8_t decimals);
	MyMessage& set(unsigned long value);
	MyMessage& set(long value);
	MyMessage& set(unsigned int value);
	MyMessage& set(int value);

#else

typedef union {
struct
{

#endif
	// Network header
#ifdef MYSENSORS_RF_NRF24
	NetworkHeaderDefault header;
#endif
	/// Message payload
	union {
		CmdVar var;
		CmdInternal internal;
		CmdFirmware firmware;
	};

#ifdef __cplusplus
} __attribute__((packed));
#else
};
uint8_t array[HEADER_SIZE + MAX_PAYLOAD + 1];	
} __attribute__((packed)) MyMessage;
#endif

#endif


