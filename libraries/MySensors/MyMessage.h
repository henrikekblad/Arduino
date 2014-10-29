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
#define NETWORK_HEADER_SIZE 7									// SET/REQ command header size
#define MAX_PAYLOAD (MAX_MESSAGE_LENGTH - NETWORK_HEADER_SIZE)	// Max payload for SET/REQ commands


// Some help defines to make sketch code more readablein sketches
#define ARMED 1
#define DISARMED 0
#define ARM 1
#define DISARM 0
#define ON 1
#define OFF 0
#define TRIPPED 1
#define UNTRIPPED 0
#define UP 1
#define DOWN 0
#define LOCKED 1
#define UNLOCKED 0
#define LOCK 1
#define UNLOCK 0


/// Command types
typedef enum {
	/// Node command sent to controller when sketch calls setup()
	CMD_NODE,

	/// Firmware OTA transmission commands.
	CMD_FIRMWARE_CONFIG_REQUEST=10,
	CMD_FIRMWARE_CONFIG_RESPONSE,
	CMD_FIRMWARE_REQUEST,
	CMD_FIRMWARE_RESPONSE,

	/// Used to report sketch or gateway version to controller. sendSketchInfo()
	CMD_VERSION,
	/// Used to report sketch name to controller. sendSketchInfo()
	CMD_NAME,
	/// Sent from gateway to controller when it's ready for action.
	CMD_GATEWAY_READY,
	/// Present devices attached to a node. Payload is the ChildSensorType.
	CMD_PRESENTATION,
	/// Request a new id from controller.
	CMD_ID,
	/// Broadcased message from a node to request neighbouring repeaters and gateway to
	/// report their distance to controller back.
	CMD_FIND_PARENT,
	/// Send in a log message to controllers application log.
	CMD_LOG_MESSAGE,

	// Report battery level. sendBatteryLevel()
	CMD_BATTERY_LEVEL,
	/// Request time from controller. requestTime(). Reply in seconds since 1970.
	CMD_TIME,

	/// Reebot node command. Requires special bootloader on the arduino.
	CMD_RESET,

	/// Activate/deactivate inclusion mode. Used between gateway and controller
	CMD_INCLUSION_MODE,

	///
	/// Below follows device related commands
	///

	/// Send RGB(W) value for led light
	DEV_RGB,
	DEV_RGBW,

	/// Scene command (turns on/off a scene on controller)
	DEV_SCENE,

	/// Send a binary state commands.
	DEV_TRIPPED,
	DEV_ARMED,
	DEV_STATUS,
	DEV_LOCKED,

	/// Send watt and kwh
	DEV_POWER,

	// Send a percentage value for things like window cover position, dimmable light and uncalibrated light levels
	DEV_PERCENTAGE,

	// Send a level value from or to a device.
	DEV_LEVEL,

	// Send or request config parameters
	DEV_CONFIG,
	// Send or request device variables
	DEV_VAR,

	// Stop command which can interrupt motion of blinds or window cover
	DEV_STOP,

	// Ackumelated value for sensor e.g. rain, water meter
	DEV_ACCUMULATED,

	// Rate values e.g. rain
	DEV_RATE,

	// Set mode for the device (different meaning for each device)
	DEV_MODE,

	// Angle report (e.g. compass, wind)
	DEV_ANGLE,

	// Send or received IR command
	DEV_IR_SEND,
	DEV_IR_RECEIVED

};
typedef uint8_t MySensorCommand;



/*

struct DoorSensor : BinaryCommand, SecurityCommand { };

struct WindowSensor : BinaryCommand, SecurityCommand { };

struct MotionSensor : BinaryCommand, SecurityCommand { };
*/


/**
 * The devices always report their data in SI units.
 * All supported commands is listed per device
 * CmdSensorVar and CmdSensorConfig is supported by all device types.
 *
 * (*) Means that sensor has different modes. Binary/Normal, Calibrated/Uncalibrated.
 * These modes is sent in with the presenttion message.
 * Binary sensors acts as security sensors and send in a tripped value when
 * some predefined criteria is meet.
 * Commands marked (B) is used in binary mode and (N) in normal mode
 * Commands makred (C) can send calibrated or (U) uncalibrated values.
 *
 */
typedef enum {
	/// Door sensor
	/// CmdSensorTripped
	/// CmdSensorArmed
	S_DOOR,

	/// Window sensor
	/// CmdSensorTripped
	/// CmdSensorArmed
	S_WINDOW,

	/// Motion sensor
	/// CmdSensorTripped
	/// CmdSensorArmed
	S_MOTION,

	/// Smoke sensor
	/// CmdSensorTripped
	/// CmdSensorArmed
	S_SMOKE,

	/// Water leak sensor
	/// CmdSensorTripped
	/// CmdSensorArmed
	S_WATER_LEAK,

	/// Binary on/off light
	/// CmdSensorState
	/// CmdSensorPower
	S_LIGHT,

	/// Binary switch sensor
	/// CmdSensorState
	S_BINARY_SWITCH,

	/// Rotary switch sensor. E.g. rotary encoder which can be turned or clicked
	/// CmdSensorTripped - Tripped value is send when clicking encoder (when supported)
	/// CmdSensorLevel
	S_ROTARY_ENCODER_SENSOR,

	/// Rotary potentiometer sensor. This sensor has end stops.
	/// CmdSensorPercentage - Sketch recalculate potentiometer value to a number between 0-100
	S_POTENTIOMETER_SENSOR,

	/// Controllable acutators that not match the light device
	/// CmdSensorState
	S_SWITCH,

	/// Dimmable actuator
	/// CmdSensorState
	/// CmdSensorPercentage
	/// CmdSensorPower
	S_DIMMABLE,

	/// RGB Light (with red, green, blue component)
	/// CmdSensorState
	/// CmdSensorRGB
	/// CmdSensorPower
	S_RGB,

	/// RGBW Light (with red, green, blue white component)
	/// CmdSensorState
	/// CmdSensorRGBW
	/// CmdSensorPower
	S_RGBW,

	/// Window covers or shades
	/// CmdSensorState - 0 close, 1 open
	/// CmdSensorPercentage - 0 closed - 100 fully open
	/// CmdSensorStop - stops blinds or window cover in the middle of motion.
	S_WINDOW_COVER,

	/// Temperature sensor (*)
	/// CmdSensorLevel (N) - Current temperature level in degrees celsius <int or float>
	/// CmdSensorTripped (B)
	/// CmdSensorArmed (B)
	S_THERMOMETER,

	/// Humidity sensor (*)
	/// CmdSensorPercentage (N)
	/// CmdSensorTripped (B)
	/// CmdSensorArmed (B)
	S_HUMIDITY,

	/// Barometer sensor or Pressure sensor (*)
	/// CmdSensorLevel (N) - Pressure level in hPa
	/// CmdSensorMode (N) - Whether forecast. One of 0="stable", 1="sunny", 2="cloudy", 3="unstable", 4="thunderstorm" or 5="unknown"
	/// CmdSensorTripped (B)
	/// CmdSensorArmed (B)
	S_BAROMETER,

	/// Wind sensor (*)
	/// CmdSensorLevel (N) - Wind level in m/s (average wind speed during last report period)
	/// CmdSensorAngle (N) - degrees clockwise from true north <int>
	/// CmdSensorTripped (B)
	/// CmdSensorArmed (B)
	S_WIND,

	/// Rain sensor (*)
	/// CmdSensorAccumulated (N) - Accumulated rain in mm
	/// CmdSensorRate (N) - Rain rate in mm/h
	/// CmdSensorTripped (B)
	/// CmdSensorArmed (B)
	S_RAIN,

	/// UV sensor (*)
	/// CmdSensorLevel (N) - Uv Index level (0-12)
	/// CmdSensorTripped (B)
	/// CmdSensorArmed (B)
	S_UV,

	/// Weight sensor
	/// CmdSensorLevel - Weight in kg <int, float>
	S_WEIGHT_SCALE,

	/// Power measuring sensor (*)
	/// CmdSensorPower (N)
	/// CmdSensorTripped (B)
	/// CmdSensorArmed (B)
	S_POWER_METER,

	/// Thermostat (for controlling heater or cooler devices)
	/// CmdSensorState - Turn 1=On, 0=Off heater or cooler power switch.
	/// CmdSensorMode - Heater/AC mode. One of 0="Off", 1="HeatOn", 2="CoolOn", or 3="AutoChangeOver"
	/// CmdSensorLevel - Setpoint for ideal temperature in celsius degrees
	S_THERMOSTAT,

	/// Distance sensor (*)
	/// CmdSensorLevel (N) - Distance in meters <int, float>
	/// CmdSensorTripped (B)
	/// CmdSensorArmed (B)
	S_DISTANCE,

	/// Light sensor (*)
	/// CmdSensorLevel (N/C) - Light level in lux
	/// CmdSensorPercentage (N/U) - Uncalibrated light level in percentage 0-100%
	/// CmdSensorTripped (B)
	/// CmdSensorArmed (B)
	S_LIGHT_SENSOR,

	/// Water meter
	/// CmdSensorAccumulated - Accumulated water volume in m3 <int, float>
	/// CmdSensorRate - Flow rate in l/m <int or float>
	S_WATER_METER,

	/// Ph sensor (*)
	/// CmdSensorLevel (N) - Ph level using standard pH scale 0-14 <int or float>
	/// CmdSensorTripped (B)
	/// CmdSensorArmed (B)
	S_PH,

	/// Scene controller device
	/// CmdSensorScene
	S_SCENE_CONTROLLER,

	/// Sound sensor (*)
	/// CmdSensorLevel (N/C) - Calibrated sound level in db
	/// CmdSensorPercentage (N/U) - Uncalibrated sound level in percentage 0-100%
	/// CmdSensorTripped (B)
	/// CmdSensorArmed (B)
	S_SOUND,

	/// Vibration sensor (*)
	/// CmdSensorLevel (N) - vibration level in Hertz
	/// CmdSensorTripped (B)
	/// CmdSensorArmed (B)
	S_VIBRATION,

	/// Gyro sensor
	/// Here we need some kind of value types. Help needed!
	S_GYRO,

	/// Compass sensor
	/// CmdSensorAngle - degrees clockwise from true north <int>
	S_COMPASS,

	/// Lock device
	/// CmdSensorLocked - 1=Locked/Lock, 0=Unlocked/Unlock
	S_LOCK,

	/// IR sender device
	/// CmdIrSend
	S_IR_SENDER,

	/// IR receiver device
	/// CmdIrReceived
	S_IR_RECEIVER,

	/// A list of more or less common gas sensors  (*)
	/// CmdSensorLevel (N/C) - Gas level in ug/m3
	/// CmdSensorPercentage (N/U) - Uncalibrated gas level
	/// CmdSensorTripped (B)
	/// CmdSensorArmed (B)
	S_DUST=100,			// Dust sensor
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

	/// Used for devices not fitting any other sensor types.
	/// If you find yourself using this it might be a candidate for a new device type?
	S_CUSTOM=255,

};
typedef uint8_t MySensorDeviceType;


typedef enum {
	P_STRING, P_BYTE, P_INT16, P_UINT16, P_LONG32, P_ULONG32, P_CUSTOM, P_FLOAT32
};
typedef uint8_t MySensorPayloadDataType;


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


typedef struct
{
	/// Id of last node this message passed
	uint8_t last;

	/// Id of sender node (origin)
	uint8_t sender;

	// Id of destination node
	uint8_t destination;

	///   0: Request an ack - Indicator that receiver should send an ack back.
	///   1: Is ack messsage - Indicator that this is the actual ack message.
	///   2: Indication if this is a request command (e.g. request config from controller or data from other node)
	/// 3-7: Length of payload
	uint8_t flags;

	/// Command type
	uint8_t command;
} NetworkHeaderDefault;



#define FIRMWARE_BLOCK_SIZE	16

typedef struct
{
	uint8_t version;
} CmdFirmwareConfigRequest;

typedef struct
{
	uint8_t version;
	uint16_t blocks;
	uint16_t crc;
} CmdFirmwareConfigResponse;

typedef struct
{
	uint8_t version;
	uint16_t block;
} CmdFirmwareRequest;

typedef struct
{
	uint8_t version;
	uint16_t block;
	uint8_t data[FIRMWARE_BLOCK_SIZE];
} CmdFirmwareResponse;


typedef struct {
	uint8_t libraryVersion;
	uint8_t isRepeater;
} CmdNode;

typedef struct {
	/// Id of device that this message concerns.
	uint8_t device;
	/// The type of device. See table above.
	MySensorDeviceType type;
	/// Indicatior if this sensor will report as a security sensor
	/// in binary mode.
	/// 0=normal, 1=binary
	uint8_t binary;
	/// Indicator if this sensor will report as a calibrated or uncalibrated percentage (where applicable)
	/// 0=uncalibrated, 1=calibrated
	uint8_t calibrated;
} CmdPresentation;

typedef struct {
	uint8_t version[MAX_PAYLOAD];
} CmdVersionReport;

typedef struct {
	uint8_t name[MAX_PAYLOAD];
} CmdNameReport;

typedef struct {
	uint16_t requestIdentifier;
} CmdIdRequest;

typedef struct {
	uint8_t newId;
	uint16_t requestIdentifier;
} CmdIdResponse;

typedef struct {
	uint8_t distance;
} CmdFindParentResponse;

typedef struct {
	uint8_t message[MAX_PAYLOAD];
} CmdLogMessageReport;

typedef struct {
	uint8_t level;
} CmdBatteryLevelReport;

typedef struct {
	uint32_t time;
} CmdTimeResponse;

typedef struct {
	/// Payload data type
	MySensorPayloadDataType ptype;

	/// Each message can transfer a payload. We add one extra byte for string
	/// terminator \0 to be "printable" this is not transferred OTA
	/// This union is used to simplify the construction of the binary data types transferred.
	union {
		uint8_t bValue;
		uint32_t ulValue;
		int32_t lValue;
		uint16_t uiValue;
		int16_t iValue;
		struct { // Float messages
			float fValue;
			uint8_t fPrecision;   // Number of decimals when serializing
		};
		char data[MAX_PAYLOAD + 1];
	} __attribute__((packed));


} MySesnorsDynamicPayload;


typedef struct {
	/// Id of device that this message concerns.
	uint8_t device;

	MySesnorsDynamicPayload value;
} CmdSensorDynamic;

typedef CmdSensorDynamic CmdSensorLevel;
typedef CmdSensorDynamic CmdSensorAccumulated;
typedef CmdSensorDynamic CmdSensorRate;

typedef struct {
	/// Id of device that this message concerns.
	uint8_t device;

	/// Id of config or var param to set or get
	uint8_t param;

	/// Payload data type
	MySensorPayloadDataType ptype;

	/// Payload value
	MySesnorsDynamicPayload value;

} CmdParamDynamic;

typedef CmdParamDynamic CmdSensorVar;
typedef CmdParamDynamic CmdSensorConfig;


typedef struct {
	/// Id of device that this message concerns.
	uint8_t device;
	/// Red, Green, Blue and white component value.
	uint8_t r;
	uint8_t g;
	uint8_t b;
} CmdSensorRGB;

typedef struct {
	/// Id of device that this message concerns.
	uint8_t device;
	/// Red, Green, Blue and white component value.
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t w;
} CmdSensorRGBW;


typedef struct {
	/// Id of device that this message concerns.
	uint8_t device;
	/// Scene number 0-255
	uint8_t scene;
	/// Status ON(1) or OFF(0)
	uint8_t status;
} CmdSensorScene;

typedef struct {
	/// Id of device that this message concerns.
	uint8_t device;
	/// TRIPPED, UNTRIPPED
	uint8_t status;
} CmdSensorTripped;

typedef struct {
	/// Id of device that this message concerns.
	uint8_t device;
	///  ARMED, DISARMED,
	uint8_t armed;
} CmdSensorArmed;

typedef struct {
	/// Id of device that this message concerns.
	uint8_t device;
	/// Status ON, OFF
	uint8_t status;
} CmdSensorStatus;

typedef struct {
	/// Id of device that this message concerns.
	uint8_t device;
	/// Status LOCKED, UNLOCKED
	uint8_t status;
} CmdSensorLocked;

typedef struct {
	/// Id of device that this message concerns.
	uint8_t device;
} CmdSensorStop;

typedef struct {
	/// Id of device that this message concerns.
	uint8_t device;
	/// The mode this device should run in .
	uint8_t mode;
} CmdSensorMode;

typedef struct {
	/// Id of device that this message concerns.
	uint8_t device;
	/// Angle in degrees from true north 0-360 .
	uint16_t angle;
} CmdSensorAngle;


typedef struct {
	/// Id of device that this message concerns.
	uint8_t device;
	/// Current watt value
	uint8_t watt;
	/// The Accumulated kwh
	uint8_t kwh;
} CmdSensorPower;

typedef struct {
	/// Id of device that this message concerns.
	uint8_t device;
	/// A Pecentage value between 0-100%
	uint8_t percentage;
} CmdSensorPercentage;


typedef struct {
	/// Id of device that this message concerns.
	uint8_t device;
	/// For now we have to send predefined ir commands in the node. Just select which code to send or has been received.
	uint16_t code;
} CmdIrSend;
typedef CmdIrSend CmdIrReceived;




#ifdef __cplusplus
class MyMessage
{
public:
	// Constructors
	MyMessage();

	MyMessage(uint8_t device, uint8_t type);

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
	MyMessage& setDevice(uint8_t device);
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
	/// Message command payloads
	union {

		// Firmaware related conmmands
		CmdFirmwareConfigRequest firmawareConfigRequest;
		CmdFirmwareConfigResponse firmawareConfigResponse;
		CmdFirmwareRequest firmwareRequest;
		CmdFirmwareResponse firmawareResponse;

		// Internal commands used by API
		CmdNode node;
		CmdPresentation presentation;
		CmdVersionReport versonReport;
		CmdNameReport nameReport;
		CmdIdRequest idRequest;
		CmdIdResponse idResponse;
		CmdFindParentResponse findParentResponse;
		CmdLogMessageReport logMessageReport;
		CmdBatteryLevelReport batteryLevelReport;
		CmdTimeResponse timeResponse;

		// Device related commands
		CmdSensorRGB rgb;
		CmdSensorRGBW rgbw;
		CmdSensorScene scene;
		CmdSensorTripped tripped;
		CmdSensorArmed armed;
		CmdSensorStatus status;
		CmdSensorLocked locked;
		CmdSensorPower power;
		CmdSensorPercentage percentage;
		CmdSensorLevel level;
		CmdSensorAccumulated accumulated;
		CmdSensorVar var;
		CmdSensorConfig config;
		CmdSensorStop stop;
		CmdSensorRate rate;
		CmdSensorMode mode;
		CmdSensorAngle angle;
		CmdIrReceived receivedIr;
		CmdIrSend sendIr;
	}  __attribute__((packed));

#ifdef __cplusplus
} __attribute__((packed));
#else
};
uint8_t array[HEADER_SIZE + MAX_PAYLOAD + 1];	
} __attribute__((packed)) MyMessage;
#endif

#endif


