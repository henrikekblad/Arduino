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
#include <ArduinoJson.h>
#endif

#define MAX_MESSAGE_LENGTH 32
#define NETWORK_HEADER_SIZE 5
#define MAX_PAYLOAD_SIZE (MAX_MESSAGE_LENGTH - NETWORK_HEADER_SIZE)	// Max payload for message part
#define FIRMWARE_BLOCK_SIZE	16

// Statuses for MSG_DEV_TRIPPED, MSG_DEV_ARMED, MSG_DEV_STATUS, MSG_DEV_LOCKED,
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

// Modes for MSG_DEV_MODE of DEV_BAROMETER
#define BARO_STABLE 0
#define BARO_SUNNY 1
#define BARO_CLOUDY 2
#define BARO_UNSTABLE 3
#define BARO_THUNDERSTORM 4
#define BARO_UNKNOWN 5


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



/// Message types
typedef enum {

	/// Firmware OTA transmission messages.
	MSG_FIRMWARE_CONFIG_REQUEST,
	MSG_FIRMWARE_CONFIG_RESPONSE,
	MSG_FIRMWARE_REQUEST,
	MSG_FIRMWARE_RESPONSE,

	/// Sent from gateway to controller when it's ready for action. No OTA
	MSG_GATEWAY_READY,
	/// Activate/deactivate inclusion mode. Used between gateway and controller. No OTA
	MSG_INCLUSION_MODE,

	/// Node message sent to controller when sketch calls setup()
	/// MsgNode
	MSG_NODE,

	/// Used to report sketch or gateway version to controller. sendSketchInfo()
	/// MsgVersion
	MSG_VERSION,

	/// Used to report sketch name to controller. sendSketchInfo()
	/// MsgName
	MSG_NAME,

	/// Present devices attached to a node. Payload is the ChildSensorType.
	/// MsgPresentation
	MSG_PRESENTATION,

	/// Request a new id from controller.
	/// MsgIdRequest, MsgIdResponse
	MSG_ID_REQUEST,
	MSG_ID_RESPONSE,

	/// Broadcased message from a node to request neighbouring repeaters and gateway to
	/// report their distance to controller back.
	/// MsgFindParentRequest, MsgFindParentResponse
	MSG_FIND_PARENT_REQUEST,
	MSG_FIND_PARENT_RESPONSE,

	/// Send in a log message to controllers application log.
	/// MsgLogMessage
	MSG_LOG_MESSAGE,

	/// Report battery level. sendBatteryLevel()
	/// MsgBatteryLevel
	MSG_BATTERY_LEVEL,

	/// Request time from controller. requestTime(). Reply in seconds since 1970.
	/// MsgTimeRequest, MsgTimeResponse
	MSG_TIME_REQUEST,
	MSG_TIME_RESPONSE,

	/// Reebot node message. Requires special bootloader on the arduino.
	/// MsgReset
	MSG_RESET,


	/**
	 * Below follows device related messages
	 */

	/// Send RGB(W) value for led light
	MSG_DEV_RGB,
	MSG_DEV_RGBW,

	/// Scene message (turns on/off a scene on controller)
	MSG_DEV_SCENE,

	/// Send a binary state messages.
	MSG_DEV_TRIPPED,
	MSG_DEV_ARMED,
	MSG_DEV_STATUS,
	MSG_DEV_LOCKED,

	/// Send watt and kwh
	MSG_DEV_POWER,

	// Send a percentage value for things like window cover position, dimmable light and uncalibrated light levels
	MSG_DEV_PERCENTAGE,

	// Send a level value from or to a device.
	MSG_DEV_LEVEL,

	// Send or request config parameters
	MSG_DEV_CONFIG,
	// Send or request device variables
	MSG_DEV_VAR,

	// Stop message which can interrupt motion of blinds or window cover
	MSG_DEV_STOP,

	// Ackumelated value for sensor e.g. rain, water meter
	MSG_DEV_ACCUMULATED,

	// Rate values e.g. rain
	MSG_DEV_RATE,

	// Set mode for the device (different meaning for each device)
	MSG_DEV_MODE,

	// Angle report (e.g. compass, wind)
	MSG_DEV_ANGLE,

	// Send or received IR message
	MSG_DEV_IR_SEND,
	MSG_DEV_IR_RECEIVED

};
typedef uint8_t MySensorMessageType;



/**
 * The devices always report their data in SI units.
 * All supported messages is listed per device
 * MsgDeviceVar and MsgDeviceConfig is supported by all device types.
 *
 * (*) Means that sensor has different modes. Binary/Normal, Calibrated/Uncalibrated.
 * These modes is sent in with the presenttion message.
 * Binary sensors acts as security sensors and send in a tripped value when
 * some predefined criteria is meet.
 * Messages marked (B) is used in binary mode and (N) in normal mode
 * Messages makred (C) can send calibrated or (U) uncalibrated values.
 *
 */
typedef enum {
	/// Door sensor
	/// MsgDeviceTripped
	/// MsgDeviceArmed
	DEV_DOOR,

	/// Window sensor
	/// MsgDeviceTripped
	/// MsgDeviceArmed
	DEV_WINDOW,

	/// Motion sensor
	/// MsgDeviceTripped
	/// MsgDeviceArmed
	DEV_MOTION,

	/// Smoke sensor
	/// MsgDeviceTripped
	/// MsgDeviceArmed
	DEV_SMOKE,

	/// Water leak sensor
	/// MsgDeviceTripped
	/// MsgDeviceArmed
	DEV_WATER_LEAK,

	/// Binary on/off light
	/// MsgDeviceState
	/// MsgDevicePower
	DEV_LIGHT,

	/// Binary switch sensor
	/// MsgDeviceState
	DEV_BINARY_SWITCH,

	/// Rotary switch sensor. E.g. rotary encoder which can be turned or clicked
	/// MsgDeviceTripped - Tripped value is send when clicking encoder (when supported)
	/// MsgDeviceLevel
	DEV_ROTARY_ENCODER_SENSOR,

	/// Rotary potentiometer sensor. This sensor has end stops.
	/// MsgDevicePercentage - Sketch recalculate potentiometer value to a number between 0-100
	DEV_POTENTIOMETER_SENSOR,

	/// Sprinker device.
	/// MsgDeviceState
	DEV_SPRINKLER,

	/// Controllable acutators that not match the light device
	/// MsgDeviceState
	DEV_SWITCH,

	/// Dimmable actuator
	/// MsgDeviceState
	/// MsgDevicePercentage
	/// MsgDevicePower
	DEV_DIMMABLE,

	/// RGB Light (with red, green, blue component)
	/// MsgDeviceState
	/// MsgDeviceRGB
	/// MsgDevicePower
	DEV_RGB,

	/// RGBW Light (with red, green, blue white component)
	/// MsgDeviceState
	/// MsgDeviceRGBW
	/// MsgDevicePower
	DEV_RGBW,

	/// Window covers or shades
	/// MsgDeviceState - 0 close, 1 open
	/// MsgDevicePercentage - 0 closed - 100 fully open
	/// MsgDeviceStop - stops blinds or window cover in the middle of motion.
	DEV_WINDOW_COVER,

	/// Temperature sensor (*)
	/// MsgDeviceLevel (N) - Current temperature level in degrees celsius <int or float>
	/// MsgDeviceTripped (B)
	/// MsgDeviceArmed (B)
	DEV_THERMOMETER,

	/// Humidity sensor (*)
	/// MsgDevicePercentage (N)
	/// MsgDeviceTripped (B)
	/// MsgDeviceArmed (B)
	DEV_HUMIDITY,

	/// Barometer sensor or Pressure sensor (*)
	/// MsgDeviceLevel (N) - Pressure level in hPa
	/// MsgDeviceMode (N) - Whether forecast. One of 0="stable", 1="sunny", 2="cloudy", 3="unstable", 4="thunderstorm" or 5="unknown"
	/// MsgDeviceTripped (B)
	/// MsgDeviceArmed (B)
	DEV_BAROMETER,

	/// Wind sensor (*)
	/// MsgDeviceLevel (N) - Wind level in m/s (average wind speed during last report period)
	/// MsgDeviceAngle (N) - degrees clockwise from true north <int>
	/// MsgDeviceTripped (B)
	/// MsgDeviceArmed (B)
	DEV_WIND,

	/// Rain sensor (*)
	/// MsgDeviceAccumulated (N) - Accumulated rain in mm
	/// MsgDeviceRate (N) - Rain rate in mm/h
	/// MsgDeviceTripped (B)
	/// MsgDeviceArmed (B)
	DEV_RAIN,

	/// UV sensor (*)
	/// MsgDeviceLevel (N) - Uv Index level (0-12)
	/// MsgDeviceTripped (B)
	/// MsgDeviceArmed (B)
	DEV_UV,

	/// Weight sensor
	/// MsgDeviceLevel - Weight in kg <int, float>
	DEV_WEIGHT_SCALE,

	/// Power measuring sensor (*)
	/// MsgDevicePower (N)
	/// MsgDeviceTripped (B)
	/// MsgDeviceArmed (B)
	DEV_POWER_METER,

	/// Thermostat (for controlling heater or cooler devices)
	/// MsgDeviceState - Turn 1=On, 0=Off heater or cooler power switch.
	/// MsgDeviceMode - Heater/AC mode. One of 0="Off", 1="HeatOn", 2="CoolOn", or 3="AutoChangeOver"
	/// MsgDeviceLevel - Setpoint for ideal temperature in celsius degrees
	DEV_THERMOSTAT,

	/// Distance sensor (*)
	/// MsgDeviceLevel (N) - Distance in meters <int, float>
	/// MsgDeviceTripped (B)
	/// MsgDeviceArmed (B)
	DEV_DISTANCE,

	/// Light sensor (*)
	/// MsgDeviceLevel (N/C) - Light level in lux
	/// MsgDevicePercentage (N/U) - Uncalibrated light level in percentage 0-100%
	/// MsgDeviceTripped (B)
	/// MsgDeviceArmed (B)
	DEV_LIGHT_SENSOR,

	/// Water meter
	/// MsgDeviceAccumulated - Accumulated water volume in m3 <int, float>
	/// MsgDeviceRate - Flow rate in l/m <int or float>
	DEV_WATER_METER,

	/// Ph sensor (*)
	/// MsgDeviceLevel (N) - Ph level using standard pH scale 0-14 <int or float>
	/// MsgDeviceTripped (B)
	/// MsgDeviceArmed (B)
	DEV_PH,

	/// Scene controller device
	/// MsgDeviceScene
	DEV_SCENE_CONTROLLER,

	/// Sound sensor (*)
	/// MsgDeviceLevel (N/C) - Calibrated sound level in db
	/// MsgDevicePercentage (N/U) - Uncalibrated sound level in percentage 0-100%
	/// MsgDeviceTripped (B)
	/// MsgDeviceArmed (B)
	DEV_SOUND,

	/// Vibration sensor (*)
	/// MsgDeviceLevel (N) - vibration level in Hertz
	/// MsgDeviceTripped (B)
	/// MsgDeviceArmed (B)
	DEV_VIBRATION,

	/// Gyro sensor
	/// Here we need some kind of value types. Help needed!
	DEV_GYRO,

	/// Compass sensor
	/// MsgDeviceAngle - degrees clockwise from true north <int>
	DEV_COMPASS,

	/// Lock device
	/// MsgDeviceLocked - 1=Locked/Lock, 0=Unlocked/Unlock
	DEV_LOCK,

	/// IR sender device
	/// MsgIrSend
	DEV_IR_SENDER,

	/// IR receiver device
	/// MsgIrReceived
	DEV_IR_RECEIVER,

	/// A list of more or less common sensors  (*)
	/// MsgDeviceLevel (N/C) - Gas level in ug/m3
	/// MsgDevicePercentage (N/U) - Uncalibrated gas level
	/// MsgDeviceTripped (B)
	/// MsgDeviceArmed (B)
	DEV_ORP=100,			// Oxidation reduction potential sensor. Water quality (V/mV.).

	DEV_DUST,				// Dust sensor
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
typedef uint8_t  MySensorDeviceType;


typedef enum {
	P_STRING, P_BYTE, P_INT16, P_UINT16, P_LONG32, P_ULONG32, P_CUSTOM, P_FLOAT32
};
typedef uint8_t MySensorPayloadDataType;




struct MyNetworkHeader {
	/// Id of last node this message passed
	uint8_t last;

	/// Id of sender node (origin)
	uint8_t sender;

	// Id of destination node
	uint8_t destination;

	///   0: Request an ack - Indicator that receiver should send an ack back.
	///		 These messages is acked automatically by MySensors::process().
	///   1: Is ack messsage - Indicator that this is an ack message.
	/// 2-7: Reserved
	uint8_t flags;

	/// Message type
	MySensorMessageType messageType;

	/**
	 * Getter for the is-ack-flag.
	 *
	 * 	@return True if this is an ack message.
	 */
	inline bool isAck() const {
		return BF_GET(flags, 1, 1);
	}

	inline void setAck(bool ack) {
		BF_SET(flags, ack, 1, 1);
	}

	/**
	 * Getter for the is-request-flag. This could be a request from some other node or controller to inform
	 * sensor to report a value.
	 *
	 * 	@return True if this is a request message.
	 */
	inline bool isReqAck() const {
		return BF_GET(flags, 0, 1);
	}

	inline void setReqAck(bool reqAck) {
		BF_SET(flags, reqAck, 0, 1);
	}


};


struct MsgType {
	virtual MySensorMessageType getMessageType();
};

struct MyPayload : MsgType {
	uint8_t deviceId;
	/// 0-6: Length of payload.  .
	///   7: Request flag. Send this to request a device value from other sensor or controller.
	///      Should be replied with a normal message.
	uint8_t length_req;


	MyPayload(uint8_t _deviceId) {
		deviceId = _deviceId;
		setRequest(false);
	}

	inline bool isRequest() const {
			return BF_GET(length_req, 7, 1);
	}

	inline void setRequest(bool request)  {
			BF_SET(length_req, request, 7, 1);
	}

	inline uint8_t getLength() const {
			return BF_GET(length_req, 0, 7);
	}

	inline void setLength(uint8_t deviceId) {
			BF_SET(length_req, deviceId, 0, 7);
	}

	MySensorDeviceType getMessageType() { return 0; }


};

struct MySensorDynamicPayload : MyPayload {
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
		char data[MAX_PAYLOAD_SIZE + 1];
	} __attribute__((packed));

	char i2h(uint8_t i) const;

	// If payload is something else than P_STRING you can have the payload value converted
	// into string representation by supplying a buffer with the minimum size of
	// 2*MAX_PAYLOAD+1. This is to be able to fit hex-conversion of a full binary payload.
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


	// Setters for payload
	MyPayload& set(void* payload, uint8_t length);
	MyPayload& set(const char* value);
	MyPayload& set(uint8_t value);
	MyPayload& set(float value, uint8_t decimals);
	MyPayload& set(unsigned long value);
	MyPayload& set(long value);
	MyPayload& set(unsigned int value);
	MyPayload& set(int value);

	MySensorDynamicPayload(uint8_t deviceId = 0) : MyPayload(deviceId)  { }
	MySensorDeviceType getMessageType() { return 0; }
};


struct MsgDeviceLevel : MySensorDynamicPayload {
	MsgDeviceLevel(uint8_t deviceId = 0) : MySensorDynamicPayload(deviceId)  { }

	MySensorDeviceType getMessageType() { return MSG_DEV_LEVEL; }
};

struct MsgDeviceAccumulated : MySensorDynamicPayload {
	MsgDeviceAccumulated(uint8_t deviceId = 0) : MySensorDynamicPayload(deviceId)  { }

	MySensorDeviceType getMessageType() { return MSG_DEV_ACCUMULATED; }
};

struct MsgDeviceRate : MySensorDynamicPayload {
	MsgDeviceRate(uint8_t deviceId = 0) :  MySensorDynamicPayload(deviceId)  { }

	MySensorDeviceType getMessageType() { return MSG_DEV_RATE; }
};


struct MsgDeviceVar : MySensorDynamicPayload {
	/// Id of config or var param to set or get
	uint8_t param;

	MsgDeviceVar(uint8_t deviceId = 0) : MySensorDynamicPayload(deviceId) { }

	MySensorDeviceType getMessageType() { return MSG_DEV_VAR; }
};

struct MsgDeviceConfig : MySensorDynamicPayload {
	/// Id of config or var param to set or get
	uint8_t param;

	MsgDeviceConfig(uint8_t deviceId = 0) : MySensorDynamicPayload(deviceId) { }
	MySensorDeviceType getMessageType() { return MSG_DEV_CONFIG; }
};


struct MsgDeviceRGB : MyPayload {
	/// Red, Green and Blue component value 0-255.
	uint8_t r;
	uint8_t g;
	uint8_t b;

	MsgDeviceRGB(uint8_t deviceId = 0) : MyPayload(deviceId) { }
	MySensorDeviceType getMessageType() { return MSG_DEV_RGB; }
};

struct MsgDeviceRGBW : MyPayload {
	/// Red, Green, Blue and white component value 0-255.
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t w;

	MsgDeviceRGBW(uint8_t deviceId = 0) : MyPayload(deviceId) { }
	MySensorDeviceType getMessageType() { return MSG_DEV_RGBW; }
};

struct MsgDeviceScene :  MyPayload {
	/// Scene number 0-255
	uint8_t scene;
	/// Status ON(1) or OFF(0)
	uint8_t status;

	MsgDeviceScene(uint8_t deviceId = 0) : MyPayload(deviceId) { }
	MySensorDeviceType getMessageType() { return MSG_DEV_SCENE; }
};

struct MsgDeviceTripped : MyPayload {
	/// TRIPPED, UNTRIPPED
	uint8_t status;

	MsgDeviceTripped(uint8_t deviceId = 0) : MyPayload(deviceId) { }
	MySensorDeviceType getMessageType() { return MSG_DEV_TRIPPED; }
};

struct MsgDeviceArmed : MyPayload {
	///  ARMED, DISARMED,
	uint8_t armed;

	MsgDeviceArmed(uint8_t deviceId = 0) : MyPayload(deviceId) { }
	MySensorDeviceType getMessageType() { return MSG_DEV_ARMED; }
};

struct MsgDeviceStatus : MyPayload {
	/// Status ON, OFF
	uint8_t status;

	MsgDeviceStatus(uint8_t deviceId = 0) : MyPayload(deviceId) { }
	MySensorDeviceType getMessageType() { return MSG_DEV_STATUS; }
};

struct MsgDeviceLocked : MyPayload {
	/// Status LOCKED, UNLOCKED or LOCK, UNLOCK
	uint8_t status;

	MsgDeviceLocked(uint8_t deviceId = 0) : MyPayload(deviceId) { }
	MySensorDeviceType getMessageType() { return MSG_DEV_LOCKED; }
};

struct MsgDeviceStop : MyPayload{
	MsgDeviceStop(uint8_t deviceId = 0) : MyPayload(deviceId) { }
	MySensorDeviceType getMessageType() { return MSG_DEV_STOP; }
};

struct MsgDeviceMode : MyPayload{
	/// The mode this device should run in .
	uint8_t mode;

	MsgDeviceMode(uint8_t deviceId = 0) : MyPayload(deviceId) { }
	MySensorDeviceType getMessageType() { return MSG_DEV_MODE; }
};

struct MsgDeviceAngle : MyPayload {
	/// Angle in degrees from true north 0-360 .
	uint16_t angle;

	MsgDeviceAngle(uint8_t deviceId = 0) : MyPayload(deviceId) { }
	MySensorDeviceType getMessageType() { return MSG_DEV_ANGLE; }
};

struct MsgDevicePower : MyPayload{
	/// Current watt value
	uint8_t watt;
	/// The Accumulated kwh
	uint8_t kwh;

	MsgDevicePower(uint8_t deviceId = 0) : MyPayload(deviceId) { }
	MySensorDeviceType getMessageType() { return MSG_DEV_POWER; }
};

typedef struct MsgDevicePercentage : MyPayload {
	/// A Pecentage value between 0-100%
	uint8_t percentage;

	MsgDevicePercentage(uint8_t deviceId = 0) : MyPayload(deviceId) { }
	MySensorDeviceType getMessageType() { return MSG_DEV_PERCENTAGE; }
};


struct MsgDeviceIrSend : MyPayload {
	/// For now we have to send predefined ir messages in the node. Just select which code to send or has been received.
	uint16_t code;

	MsgDeviceIrSend(uint8_t deviceId = 0) : MyPayload(deviceId) { }
	MySensorDeviceType getMessageType() { return MSG_DEV_IR_SEND; }
};


struct MsgDeviceIrReceived : MyPayload {
	/// For now we have to send predefined ir messages in the node. Just select which code to send or has been received.
	uint16_t code;

	MsgDeviceIrReceived(uint8_t deviceId = 0) : MyPayload(deviceId) { }
	MySensorDeviceType getMessageType() { return MSG_DEV_IR_RECEIVED; }
};



/**
 * INTERNAL MESSAGES
 */


struct MsgFirmwareConfigRequest  {
	uint8_t version;
};


struct MsgFirmwareConfigResponse  {
	uint8_t version;
	uint16_t blocks;
	uint16_t crc;
};

struct MsgFirmwareRequest {
	uint8_t version;
	uint16_t block;
};

struct MsgFirmwareResponse {
	uint8_t version;
	uint16_t block;
	uint8_t data[FIRMWARE_BLOCK_SIZE];
};

struct MsgNode {
	// Library version of this node
	uint8_t majorVersion;
	uint8_t minorVersion;
	/// Is repeater enabled for this node?
	uint8_t isRepeater;
	/// Parent node
	uint8_t parent;
};

struct MsgPresentation {
	uint8_t deviceId;

	MySensorDeviceType deviceType;
	/// Indicatior if this sensor will report as a security sensor
	/// in binary mode.
	/// 0=normal, 1=binary
	uint8_t binary;
	/// Indicator if this sensor will report as a calibrated or uncalibrated percentage (where applicable)
	/// 0=uncalibrated, 1=calibrated
	uint8_t calibrated;
};


struct MsgIdRequest {
	uint16_t requestIdentifier;
};

struct MsgIdResponse {
	uint16_t requestIdentifier;
	uint8_t newId;
};


struct MsgFindParentResponse {
	uint8_t distance;
};


struct MsgBatteryLevel {
	uint8_t level;
};

struct MsgTimeResponse  {
	uint32_t time;
};


struct MyMessage  {

	MyNetworkHeader header;

	uint8_t payload[MAX_PAYLOAD_SIZE + 1];


	/// Helper for setting device id "on the fly" when calling MySensors::send()
	//	virtual ~MyMessage() {}
//	virtual MyMessage fromJson(Parser::JsonObject json) = 0;
//	virtual Generator::JsonObject toJson() = 0;
};



/*


#ifdef __cplusplus
class MyMessage
{
public:
	// Constructors
	MyMessage();

	MyMessage(uint8_t device, uint8_t type);


	union {
		// Firmaware related messagess
		MsgFirmwareConfigRequest firmawareConfigRequest;
		MsgFirmwareConfigResponse firmawareConfigResponse;
		MsgFirmwareRequest firmwareRequest;
		MsgFirmwareResponse firmawareResponse;

		// Internal messages used by API
		MsgNode node;
		MsgPresentation presentation;
		MsgVersion verson;
		MsgName name;
		MsgIdRequest idRequest;
		MsgIdResponse idResponse;
		MsgFindParentRequest findParentRequest;
		MsgFindParentResponse findParentResponse;
		MsgLogMessage logMessage;
		MsgBatteryLevel batteryLevel;
		MsgTimeRequest timeRequest;
		MsgTimeResponse timeResponse;
		MsgReset reset;

		// Device related messages (contains a device id)
		MsgDeviceRGB rgb;
		MsgDeviceRGBW rgbw;
		MsgDeviceScene scene;
		MsgDeviceTripped tripped;
		MsgDeviceArmed armed;
		MsgDeviceStatus status;
		MsgDeviceLocked locked;
		MsgDevicePower power;
		MsgDevicePercentage percentage;
		MsgDeviceLevel level;
		MsgDeviceAccumulated accumulated;
		MsgDeviceVar var;
		MsgDeviceConfig config;
		MsgDeviceStop stop;
		MsgDeviceRate rate;
		MsgDeviceMode mode;
		MsgDeviceAngle angle;
		MsgDeviceIrReceived receivedIr;
		MsgDeviceIrSend sendIr;
	}  __attribute__((packed)) payload;




#else

typedef union {
struct
{

#endif
	// Network header
	NetworkHeaderDefault header;

#ifdef __cplusplus
} __attribute__((packed));
#else
};
uint8_t array[HEADER_SIZE + MAX_PAYLOAD + 1];	
} __attribute__((packed)) MyMessage;
#endif */

#endif


