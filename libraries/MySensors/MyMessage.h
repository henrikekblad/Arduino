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
#define NETWORK_HEADER_SIZE 5
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
	CMD_DEV_RGB,
	CMD_DEV_RGBW,

	/// Scene command (turns on/off a scene on controller)
	CMD_DEV_SCENE,

	/// Send a binary state commands.
	CMD_DEV_TRIPPED,
	CMD_DEV_ARMED,
	CMD_DEV_STATUS,
	CMD_DEV_LOCKED,

	/// Send watt and kwh
	CMD_DEV_POWER,

	// Send a percentage value for things like window cover position, dimmable light and uncalibrated light levels
	CMD_DEV_PERCENTAGE,

	// Send a level value from or to a device.
	CMD_DEV_LEVEL,

	// Send or request config parameters
	CMD_DEV_CONFIG,
	// Send or request device variables
	CMD_DEV_VAR,

	// Stop command which can interrupt motion of blinds or window cover
	CMD_DEV_STOP,

	// Ackumelated value for sensor e.g. rain, water meter
	CMD_DEV_ACCUMULATED,

	// Rate values e.g. rain
	CMD_DEV_RATE,

	// Set mode for the device (different meaning for each device)
	CMD_DEV_MODE,

	// Angle report (e.g. compass, wind)
	CMD_DEV_ANGLE,

	// Send or received IR command
	CMD_DEV_IR_SEND,
	CMD_DEV_IR_RECEIVED

};
typedef uint8_t MySensorCommand;



/**
 * The devices always report their data in SI units.
 * All supported commands is listed per device
 * CmdDeviceVar and CmdDeviceConfig is supported by all device types.
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
	/// CmdDeviceTripped
	/// CmdDeviceArmed
	DEV_DOOR,

	/// Window sensor
	/// CmdDeviceTripped
	/// CmdDeviceArmed
	DEV_WINDOW,

	/// Motion sensor
	/// CmdDeviceTripped
	/// CmdDeviceArmed
	DEV_MOTION,

	/// Smoke sensor
	/// CmdDeviceTripped
	/// CmdDeviceArmed
	DEV_SMOKE,

	/// Water leak sensor
	/// CmdDeviceTripped
	/// CmdDeviceArmed
	DEV_WATER_LEAK,

	/// Binary on/off light
	/// CmdDeviceState
	/// CmdDevicePower
	DEV_LIGHT,

	/// Binary switch sensor
	/// CmdDeviceState
	DEV_BINARY_SWITCH,

	/// Rotary switch sensor. E.g. rotary encoder which can be turned or clicked
	/// CmdDeviceTripped - Tripped value is send when clicking encoder (when supported)
	/// CmdDeviceLevel
	DEV_ROTARY_ENCODER_SENSOR,

	/// Rotary potentiometer sensor. This sensor has end stops.
	/// CmdDevicePercentage - Sketch recalculate potentiometer value to a number between 0-100
	DEV_POTENTIOMETER_SENSOR,

	/// Controllable acutators that not match the light device
	/// CmdDeviceState
	DEV_SWITCH,

	/// Dimmable actuator
	/// CmdDeviceState
	/// CmdDevicePercentage
	/// CmdDevicePower
	DEV_DIMMABLE,

	/// RGB Light (with red, green, blue component)
	/// CmdDeviceState
	/// CmdDeviceRGB
	/// CmdDevicePower
	DEV_RGB,

	/// RGBW Light (with red, green, blue white component)
	/// CmdDeviceState
	/// CmdDeviceRGBW
	/// CmdDevicePower
	DEV_RGBW,

	/// Window covers or shades
	/// CmdDeviceState - 0 close, 1 open
	/// CmdDevicePercentage - 0 closed - 100 fully open
	/// CmdDeviceStop - stops blinds or window cover in the middle of motion.
	DEV_WINDOW_COVER,

	/// Temperature sensor (*)
	/// CmdDeviceLevel (N) - Current temperature level in degrees celsius <int or float>
	/// CmdDeviceTripped (B)
	/// CmdDeviceArmed (B)
	DEV_THERMOMETER,

	/// Humidity sensor (*)
	/// CmdDevicePercentage (N)
	/// CmdDeviceTripped (B)
	/// CmdDeviceArmed (B)
	DEV_HUMIDITY,

	/// Barometer sensor or Pressure sensor (*)
	/// CmdDeviceLevel (N) - Pressure level in hPa
	/// CmdDeviceMode (N) - Whether forecast. One of 0="stable", 1="sunny", 2="cloudy", 3="unstable", 4="thunderstorm" or 5="unknown"
	/// CmdDeviceTripped (B)
	/// CmdDeviceArmed (B)
	DEV_BAROMETER,

	/// Wind sensor (*)
	/// CmdDeviceLevel (N) - Wind level in m/s (average wind speed during last report period)
	/// CmdDeviceAngle (N) - degrees clockwise from true north <int>
	/// CmdDeviceTripped (B)
	/// CmdDeviceArmed (B)
	DEV_WIND,

	/// Rain sensor (*)
	/// CmdDeviceAccumulated (N) - Accumulated rain in mm
	/// CmdDeviceRate (N) - Rain rate in mm/h
	/// CmdDeviceTripped (B)
	/// CmdDeviceArmed (B)
	DEV_RAIN,

	/// UV sensor (*)
	/// CmdDeviceLevel (N) - Uv Index level (0-12)
	/// CmdDeviceTripped (B)
	/// CmdDeviceArmed (B)
	DEV_UV,

	/// Weight sensor
	/// CmdDeviceLevel - Weight in kg <int, float>
	DEV_WEIGHT_SCALE,

	/// Power measuring sensor (*)
	/// CmdDevicePower (N)
	/// CmdDeviceTripped (B)
	/// CmdDeviceArmed (B)
	DEV_POWER_METER,

	/// Thermostat (for controlling heater or cooler devices)
	/// CmdDeviceState - Turn 1=On, 0=Off heater or cooler power switch.
	/// CmdDeviceMode - Heater/AC mode. One of 0="Off", 1="HeatOn", 2="CoolOn", or 3="AutoChangeOver"
	/// CmdDeviceLevel - Setpoint for ideal temperature in celsius degrees
	DEV_THERMOSTAT,

	/// Distance sensor (*)
	/// CmdDeviceLevel (N) - Distance in meters <int, float>
	/// CmdDeviceTripped (B)
	/// CmdDeviceArmed (B)
	DEV_DISTANCE,

	/// Light sensor (*)
	/// CmdDeviceLevel (N/C) - Light level in lux
	/// CmdDevicePercentage (N/U) - Uncalibrated light level in percentage 0-100%
	/// CmdDeviceTripped (B)
	/// CmdDeviceArmed (B)
	DEV_LIGHT_SENSOR,

	/// Water meter
	/// CmdDeviceAccumulated - Accumulated water volume in m3 <int, float>
	/// CmdDeviceRate - Flow rate in l/m <int or float>
	DEV_WATER_METER,

	/// Ph sensor (*)
	/// CmdDeviceLevel (N) - Ph level using standard pH scale 0-14 <int or float>
	/// CmdDeviceTripped (B)
	/// CmdDeviceArmed (B)
	DEV_PH,

	/// Scene controller device
	/// CmdDeviceScene
	DEV_SCENE_CONTROLLER,

	/// Sound sensor (*)
	/// CmdDeviceLevel (N/C) - Calibrated sound level in db
	/// CmdDevicePercentage (N/U) - Uncalibrated sound level in percentage 0-100%
	/// CmdDeviceTripped (B)
	/// CmdDeviceArmed (B)
	DEV_SOUND,

	/// Vibration sensor (*)
	/// CmdDeviceLevel (N) - vibration level in Hertz
	/// CmdDeviceTripped (B)
	/// CmdDeviceArmed (B)
	DEV_VIBRATION,

	/// Gyro sensor
	/// Here we need some kind of value types. Help needed!
	DEV_GYRO,

	/// Compass sensor
	/// CmdDeviceAngle - degrees clockwise from true north <int>
	DEV_COMPASS,

	/// Lock device
	/// CmdDeviceLocked - 1=Locked/Lock, 0=Unlocked/Unlock
	DEV_LOCK,

	/// IR sender device
	/// CmdIrSend
	DEV_IR_SENDER,

	/// IR receiver device
	/// CmdIrReceived
	DEV_IR_RECEIVER,

	/// A list of more or less common gas sensors  (*)
	/// CmdDeviceLevel (N/C) - Gas level in ug/m3
	/// CmdDevicePercentage (N/U) - Uncalibrated gas level
	/// CmdDeviceTripped (B)
	/// CmdDeviceArmed (B)
	DEV_DUST=100,			// Dust sensor
	DEV_CARBON_MONOXIDE, 	// Carbon Monoxide – CO
	DEV_CARBON_DIOXIDE, 	// Carbon Dioxide – CO2
	DEV_OXYGENE, 			// Oxygen – O2
	DEV_METHANE,			// Methane – CH4
	DEV_HYDROGEN,			// Hydrogen – H2
	DEV_AMMONIA,			// Ammonia – NH3
	DEV_ISOBUTANE, 		// Isobutane – C4H10
	DEV_ETHANOL,			// Ethanol – CH3CH2OH (aka C2H5OH or C2H60)
	DEV_ETHANOL_50,		// Ethanol CO2H50H
	DEV_TOULENE,			// Toluene – C6H5CH3
	DEV_HYDROGEN_SULFIDE, // Hydrogen Sulfide – H2S
	DEV_NITROGEN_DIOXIDE, // Nitrogen Dioxide – NO2
	DEV_OZONE,			// Ozone – O3
	DEV_HYDROCARBON,		// Hydrocarbons – VOC
	DEV_CHLORINE,			// Chlorine CL2
	DEV_AMMONIUM,			// Ammonium NH4
	DEV_METHYL,			// Methyl CH3
	DEV_ACETONE, 			// Acetone CH3_2CO
	DEV_LPG,				// LPG (both C3H8 C4H10)
	DEV_NITRIC_OXIDE,		// Nitric oxide NO
	DEV_NITROGENE_OXIDES,	// Nitrogen oxides NOX
	DEV_NO3,				// NO3 ion (nitrate, not adjusted for ammonium ion) UG/M3
	DEV_S04,				// SO4 ion, sulfate, not adjusted for ammonium ion, UG/M3
	DEV_S02,				// SO2 sulfur dioxide, ppb
	DEV_EC,				// EC (elemental carbon) – PM2.5 UG/M3
	DEV_ORGANIC_CARBON,	// OC (organic carbon, not adjusted for oxygen and hydrogen) – PM2.5 UG/M3
	DEV_BLACK_CARBON, 	// BC (black carbon at 880 nm) UG/M3
	DEV_UV_AETH,			// UV-AETH (second channel of Aethalometer at 370 nm) UG/M3
	DEV_PM2,				// PM2.5 mass - UG/M3
	DEV_PM10,				// PM10 mass - PM10 mass

	/// Used for devices not fitting any other sensor types.
	/// If you find yourself using this it might be a candidate for a new device type?
	DEV_CUSTOM=255,

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
	///		 These messages should be replied with the same message/payload and the IsAckMessage bit set.
	///   1: Is ack messsage - Indicator that this is an ack message.
	///   2: Indication if this is a request command (e.g. request config from controller or data from node)
	///		 They should be replied with the actual value
	/// 3-7: Reserved
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
} CmdDeviceDynamic;

typedef CmdDeviceDynamic CmdDeviceLevel;
typedef CmdDeviceDynamic CmdDeviceAccumulated;
typedef CmdDeviceDynamic CmdDeviceRate;

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

typedef CmdParamDynamic CmdDeviceVar;
typedef CmdParamDynamic CmdDeviceConfig;


typedef struct {
	/// Id of device that this message concerns.
	uint8_t device;
	/// Red, Green, Blue and white component value.
	uint8_t r;
	uint8_t g;
	uint8_t b;
} CmdDeviceRGB;

typedef struct {
	/// Id of device that this message concerns.
	uint8_t device;
	/// Red, Green, Blue and white component value.
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t w;
} CmdDeviceRGBW;


typedef struct {
	/// Id of device that this message concerns.
	uint8_t device;
	/// Scene number 0-255
	uint8_t scene;
	/// Status ON(1) or OFF(0)
	uint8_t status;
} CmdDeviceScene;

typedef struct {
	/// Id of device that this message concerns.
	uint8_t device;
	/// TRIPPED, UNTRIPPED
	uint8_t status;
} CmdDeviceTripped;

typedef struct {
	/// Id of device that this message concerns.
	uint8_t device;
	///  ARMED, DISARMED,
	uint8_t armed;
} CmdDeviceArmed;

typedef struct {
	/// Id of device that this message concerns.
	uint8_t device;
	/// Status ON, OFF
	uint8_t status;
} CmdDeviceStatus;

typedef struct {
	/// Id of device that this message concerns.
	uint8_t device;
	/// Status LOCKED, UNLOCKED
	uint8_t status;
} CmdDeviceLocked;

typedef struct {
	/// Id of device that this message concerns.
	uint8_t device;
} CmdDeviceStop;

typedef struct {
	/// Id of device that this message concerns.
	uint8_t device;
	/// The mode this device should run in .
	uint8_t mode;
} CmdDeviceMode;

typedef struct {
	/// Id of device that this message concerns.
	uint8_t device;
	/// Angle in degrees from true north 0-360 .
	uint16_t angle;
} CmdDeviceAngle;


typedef struct {
	/// Id of device that this message concerns.
	uint8_t device;
	/// Current watt value
	uint8_t watt;
	/// The Accumulated kwh
	uint8_t kwh;
} CmdDevicePower;

typedef struct {
	/// Id of device that this message concerns.
	uint8_t device;
	/// A Pecentage value between 0-100%
	uint8_t percentage;
} CmdDevicePercentage;


typedef struct {
	/// Id of device that this message concerns.
	uint8_t device;
	/// For now we have to send predefined ir commands in the node. Just select which code to send or has been received.
	uint16_t code;
} CmdDeviceIrSend;
typedef CmdDeviceIrSend CmdDeviceIrReceived;




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
	/// Command payloads
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

		// Device related commands (contains a device id)
		CmdDeviceRGB rgb;
		CmdDeviceRGBW rgbw;
		CmdDeviceScene scene;
		CmdDeviceTripped tripped;
		CmdDeviceArmed armed;
		CmdDeviceStatus status;
		CmdDeviceLocked locked;
		CmdDevicePower power;
		CmdDevicePercentage percentage;
		CmdDeviceLevel level;
		CmdDeviceAccumulated accumulated;
		CmdDeviceVar var;
		CmdDeviceConfig config;
		CmdDeviceStop stop;
		CmdDeviceRate rate;
		CmdDeviceMode mode;
		CmdDeviceAngle angle;
		CmdDeviceIrReceived receivedIr;
		CmdDeviceIrSend sendIr;
	}  __attribute__((packed));

#ifdef __cplusplus
} __attribute__((packed));
#else
};
uint8_t array[HEADER_SIZE + MAX_PAYLOAD + 1];	
} __attribute__((packed)) MyMessage;
#endif

#endif


