/**
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */

#include "MySensor.h"

#define DISTANCE_INVALID (0xFF)


// Inlined helper functions

// Route device messages
/*static inline bool route(MySensor &ms, MyMessage &msg, const uint8_t deviceId, MyPayload &payload, const bool requestAck, const bool isAck, const bool requestData, const uint8_t destination) {
	msg.header.messageType = payload.getDeviceType();
	msg.header.sender = ms.getNodeId();
    msg.header.setReqAck(requestAck);
	msg.header.setAck(isAck);
	msg.header.destination = destination;
	payload.setRequest(requestData);
	//((MyPayload *)msg->payload)->deviceId = deviceId;
	memcpy(msg.payload, &payload, payload.getLength() + sizeof(MyPayload));
	return ms.sendRoute(msg, sizeof(MyNetworkHeader) + ((MyPayload *)payload)->getLength() + sizeof(MyPayload) );
}*/

// Route internal messages
static inline bool route(MySensor &ms, MyMessage &msg, const uint8_t length, const uint8_t messageType, const bool requestAck, const bool isAck, const uint8_t destination) {
	msg.header.messageType = messageType;
	msg.header.sender = ms.getNodeId();
    msg.header.setReqAck(requestAck);
	msg.header.setAck(isAck);
	msg.header.destination = destination;
	return ms.sendRoute(msg, sizeof(MyNetworkHeader) + length);
}


static inline bool isValidParent( const uint8_t parent ) {
	return parent != AUTO;
}
static inline bool isValidDistance( const uint8_t distance ) {
	return distance != DISTANCE_INVALID;
}


MySensor::MySensor() {
	driver = (MyDriver*) new MyDriverClass();
}

void MySensor::begin(void (*_msgCallback)(const MyMessage &), uint8_t _nodeId, bool _repeaterMode, uint8_t _parentNodeId) {
	Serial.begin(BAUD_RATE);
	repeaterMode = _repeaterMode;
	msgCallback = _msgCallback;
	failedTransmissions = 0;
	// Only gateway should use node id 0
	isGateway = _nodeId == 0;

	if (repeaterMode) {
		childNodeTable = new uint8_t[256];
		eeprom_read_block((void*)childNodeTable, (void*)EEPROM_ROUTES_ADDRESS, 256);
	}

	failedTransmissions = 0;
	driver->init();

	// Read settings from EEPROM
	eeprom_read_block((void*)&nc, (void*)EEPROM_NODE_ID_ADDRESS, sizeof(NodeConfig));

	if (isGateway) {
		nc.distance = 0;
	}

	autoFindParent = _parentNodeId == AUTO;
	if (!autoFindParent) {
		nc.parentNodeId = _parentNodeId;
		nc.distance = 0;
	} else if (!isValidParent(nc.parentNodeId)) {
		// Auto find parent, but parent in eeprom is invalid. Force parent search on first transmit.
		nc.distance = DISTANCE_INVALID;
	}

	if (_nodeId != AUTO) {
		// Set static id
		nc.nodeId = _nodeId;
	}

	// Try to fetch node-id from gateway
	if (nc.nodeId == AUTO) {
		requestNodeId();
	}

	if (!isGateway) {
		debug(PSTR("%s started, id=%d, parent=%d, distance=%d\n"), repeaterMode?"repeater":"sensor", nc.nodeId, nc.parentNodeId, nc.distance);
	}

	// Open reading pipe for messages directed to this node (set write pipe to same)
	driver->setAddress(nc.nodeId);

	// Send node presentation to controller.
	((MsgNode *)msg.payload)->isRepeater = repeaterMode;
	((MsgNode *)msg.payload)->majorVersion = MAJOR_VERSION;
	((MsgNode *)msg.payload)->minorVersion = MINOR_VERSION;
	((MsgNode *)msg.payload)->parent = nc.parentNodeId;
	route(*this, msg, sizeof(MsgNode), MSG_NODE, false, false, GATEWAY_ADDRESS);

	// Wait configuration reply.
	waitForReply();
}



uint8_t MySensor::getNodeId() {
	return nc.nodeId;
}


void MySensor::requestNodeId() {
	debug(PSTR("req node id\n"));
	driver->setAddress(nc.nodeId);
	((MsgIdRequest *)msg.payload)->requestIdentifier = requestIdentifier = (uint16_t) micros(); // generate "random" number. Trucate high bits.
	route(*this, msg, MSG_ID_REQUEST, sizeof(MsgIdRequest), false, false, GATEWAY_ADDRESS);
	waitForReply();
}


void MySensor::findParentNode() {
	// Send ping message to BROADCAST_ADDRESS (to which all relaying nodes and gateway listens and should reply to)
	debug(PSTR("find parent\n"));
	route(*this, msg, 0, MSG_FIND_PARENT_REQUEST, false, false, BROADCAST_ADDRESS);
	// Wait for ping response.
	waitForReply();
}

void MySensor::waitForReply() {
	unsigned long enter = millis();
	// Let's process incoming messages for a couple of seconds
	while (millis() - enter < 2000) {
		process();
	}
}

bool MySensor::sendRoute(MyMessage &message, const uint8_t length) {
	// If we still don't have any node id, re-request and skip this message.
	if (nc.nodeId == AUTO && !(message.header.messageType == MSG_ID_REQUEST)) {
		requestNodeId();
		return false;
	}

	if (repeaterMode) {
		uint8_t dest = message.header.destination;
		uint8_t route = getChildRoute(dest);
		if (route>GATEWAY_ADDRESS && route<BROADCAST_ADDRESS && dest != GATEWAY_ADDRESS) {
			// --- debug(PSTR("route %d.\n"), route);
			// Message destination is not gateway and is in routing table for this node.
			// Send it downstream
			return sendWrite(route, message, length);
		} else if (message.header.messageType == MSG_ID_RESPONSE && dest==BROADCAST_ADDRESS) {
			// Node has not yet received any id. We need to send it
			// by doing a broadcast sending,
			return sendWrite(BROADCAST_ADDRESS, message, length);
		}
	}

	if (!isGateway) {
		// Should be routed back to gateway.
		return sendWrite(nc.parentNodeId, message, length);
	}
	return false;
}

bool MySensor::sendWrite(const uint8_t next, MyMessage &message,  uint8_t length, bool allowFindParent) {
	bool ok = true;
	const bool broadcast = next == BROADCAST_ADDRESS;
	const bool toParent  = next == nc.parentNodeId;
	// With current implementation parent node Id can equal the broadcast address when
	// starting with empty eeprom and AUTO node Id is active.
	// This behavior is undesired, as possible parents will report back to broadcast address.
//	debug(PSTR("sendWrite next=%d, parent=%d, distance=%d\n"), next, nc.parentNodeId, nc.distance);
	// If sending directly to parent node and distance is not set, then try to find parent now.
	if ( allowFindParent && toParent && !isValidDistance(nc.distance) ) {
		findParentNode();
		// Known distance indicates parent has been found
		ok = isValidDistance(nc.distance);
	}

	if (ok) {
		message.header.last = nc.nodeId;
		bool ok = driver->send(next, &message, length);

/*		debug(PSTR("send: %d-%d-%d-%d s=%d,c=%d,t=%d,pt=%d,l=%d,st=%s:%s\n"),
				message.sender,message.last, next, message.destination, message.deviceId, message.messageType, message.messageType,
				mGetPayloadType(message), mGetLength(message), broadcast ? "bc" : (ok ? "ok":"fail"), message.getString(convBuf));
		*/

		// If many successive transmissions to parent failed, the parent node might be down and we
		// need to find another route to gateway.
		if (toParent) {
			if (ok) {
				failedTransmissions = 0;
			} else {
				failedTransmissions++;
				if ( autoFindParent && (failedTransmissions >= SEARCH_FAILURES)) {
					debug(PSTR("lost parent\n"));
					// Set distance invalid to trigger parent search on next write.
					nc.distance = DISTANCE_INVALID;
					failedTransmissions = 0;
				}
			}
		}
	}
	return ok;
}



bool MySensor::send(MyPayload &pl, const uint8_t destination, const bool requestAck) {
//	return route(*this, msg, pl.deviceId, pl, requestAck, false, false, destination);

	msg.header.messageType = pl.getMessageType();
	msg.header.sender = nc.nodeId;
	msg.header.setReqAck(requestAck);
	msg.header.setAck(false);
	msg.header.destination = destination;
	//((MyPayload *)msg->payload)->deviceId = deviceId;
	memcpy(msg.payload, &pl, pl.getLength() + sizeof(MyPayload));
	((MyPayload *)msg.payload)->setRequest(false);
	return sendRoute(msg, sizeof(MyNetworkHeader) + sizeof(MyPayload) + pl.getLength()  );

}

bool MySensor::request(const uint8_t deviceId, const MySensorMessageType messageType, const uint8_t destination) {
	// No payload for request data messages
	((MyPayload *)msg.payload)->setLength(sizeof(MyPayload));
	//msg.setDe(sizeof(MyPayload));
	((MyPayload *)msg.payload)->setRequest(true);

	msg.header.messageType = messageType;
	msg.header.sender = nc.nodeId;
	msg.header.setReqAck(false);
	msg.header.setAck(false);
	msg.header.destination = destination;
	return sendRoute(msg, sizeof(MyNetworkHeader) + sizeof(MyPayload) );


	//return route(*this, msg, deviceId, payload, deviceType, false, false, true, destination);
}


bool MySensor::sendBatteryLevel(uint8_t value, bool requestAck) {
	((MsgBatteryLevel *)msg.payload)->level = value;
	return route(*this, msg, sizeof(MsgBatteryLevel), MSG_BATTERY_LEVEL, requestAck, false, GATEWAY_ADDRESS);
}

bool MySensor::present(uint8_t deviceId, MySensorDeviceType deviceType, bool binary, bool calibrated, bool requestAck) {
	((MsgPresentation *)msg.payload)->deviceId = deviceId;
	((MsgPresentation *)msg.payload)->deviceType = deviceType;
	((MsgPresentation *)msg.payload)->calibrated = calibrated;
	((MsgPresentation *)msg.payload)->binary = binary;
	return route(*this, msg, sizeof(MsgPresentation),  MSG_PRESENTATION, requestAck, false, GATEWAY_ADDRESS);
}

bool MySensor::logMessage(const char *message, bool requestAck) {
	uint8_t len = min(MAX_PAYLOAD_SIZE,strlen(message));
	strncpy((char *)msg.payload,message, len);
	return route(*this, msg, len, MSG_LOG_MESSAGE, requestAck, false, GATEWAY_ADDRESS);
}


bool MySensor::sendSketchInfo(const char *name, const char *version, bool requstAck) {
	bool ok = true;
	if (name != NULL) {
		strcpy((char *)msg.payload,name);
		ok = route(*this, msg, strlen(name), MSG_NAME, requstAck, false, GATEWAY_ADDRESS);
	}
    if (version != NULL) {
		strcpy((char *)msg.payload, version);
    	return ok && route(*this, msg, strlen(version), MSG_VERSION, requstAck, false, GATEWAY_ADDRESS);
    }
    return ok;
}


bool MySensor::requestTime(void (* _timeCallback)(unsigned long)) {
	timeCallback = _timeCallback;
	return route(*this, msg, 0,  MSG_TIME_REQUEST, false, false, GATEWAY_ADDRESS);
}


bool MySensor::process() {
	uint8_t to = 0;
	if (!driver->available(&to))
		return false;

	uint8_t len = driver->receive((uint8_t *)&msg);

	// Add string termination, good if we later would want to print it.
	//msg.payload[mGetLength(msg)] = '\0';
/*	debug(PSTR("read: %d-%d-%d s=%d,c=%d,t=%d,pt=%d,l=%d:%s\n"),
				msg.header.sender, msg.header.last, msg.header.destination,  msg.deviceId,  msg.header.messageType, mGetPayloadType(msg), mGetLength(msg), msg.getString(convBuf));
*/
	/*if(!(mGetVersion(msg) == PROTOCOL_VERSION)) {
		debug(PSTR("version mismatch\n"));
		return false;
	}*/

	uint8_t type = msg.header.messageType;
	uint8_t sender = msg.header.sender;
	uint8_t last = msg.header.last;
	uint8_t destination = msg.header.destination;

	if (repeaterMode && type == MSG_FIND_PARENT_REQUEST) {
		// Relaying nodes should always answer ping messages
		// Wait a random delay of 0-1.023 seconds to minimize collision
		// between ping ack messages from other relaying nodes
		delay(millis() & 0x3ff);
		((MsgFindParentResponse *)msg.payload)->distance = nc.distance;
		route(*this, msg, sizeof(MsgFindParentResponse), MSG_FIND_PARENT_RESPONSE, false, false, sender);
		return false;
	} else if (type == MSG_TIME_RESPONSE) {
		if (timeCallback != NULL) {
			// Deliver time to callback
			timeCallback(((MsgTimeResponse *)msg.payload)->time);
		}
	} else if (destination == nc.nodeId) {
		// Check if sender requests an ack back.
		if (msg.header.isReqAck()) {
			// Copy message
			ack = msg;
			ack.header.setReqAck(false); // Reply without ack flag (otherwise we would end up in an eternal loop)
			ack.header.setAck(true);
			ack.header.sender = nc.nodeId;
			ack.header.destination = msg.header.sender;
			sendRoute(ack, len);
		}

		// This message is addressed to this node
		if (repeaterMode && last != nc.parentNodeId) {
			// Message is from one of the child nodes. Add it to routing table.
			addChildRoute(sender, last);
		}

		if (type == MSG_FIND_PARENT_RESPONSE && !isGateway) {
			// We've received a reply to a FIND_PARENT message. Check if the distance is
			// shorter than we already have.
			uint8_t distance = ((MsgFindParentResponse *)msg.payload)->distance;
			if (isValidDistance(distance))
			{
				// Distance to gateway is one more for us w.r.t. parent
				distance++;
				if (isValidDistance(distance) && (distance < nc.distance)) {
					// Found a neighbor closer to GW than previously found
					nc.distance = distance;
					nc.parentNodeId = msg.header.sender;
					eeprom_write_byte((uint8_t*)EEPROM_PARENT_NODE_ID_ADDRESS, nc.parentNodeId);
					eeprom_write_byte((uint8_t*)EEPROM_DISTANCE_ADDRESS, nc.distance);
					debug(PSTR("new parent=%d, d=%d\n"), nc.parentNodeId, nc.distance);
				}
			}
			return false;
		} else if (sender == GATEWAY_ADDRESS) {
			if (type == MSG_RESET) {
				// Requires MySensors or other bootloader with watchdogs enabled
				wdt_enable(WDTO_15MS);
				for (;;);
			} else if (type == MSG_ID_RESPONSE && nc.nodeId == AUTO && ((MsgIdResponse *)msg.payload)->requestIdentifier == requestIdentifier) {
				nc.nodeId = ((MsgIdResponse *)msg.payload)->newId;
				// Write id to EEPROM
				if (nc.nodeId == AUTO) {
					// sensor net gateway will return max id if all sensor id are taken
					debug(PSTR("full\n"));
					while (1); // Wait here. Nothing else we can do...
				} else {
					driver->setAddress(nc.nodeId);
					eeprom_write_byte((uint8_t*)EEPROM_NODE_ID_ADDRESS, nc.nodeId);
				}
				debug(PSTR("id=%d\n"), nc.nodeId);
			}
			return false;
		}

		// Call incoming message callback if available
		if (msgCallback != NULL) {
			msgCallback(msg);
		}
		// Return true if message was addressed for this node...
		return true;
	} else if (repeaterMode && to == nc.nodeId) {
		// We should try to relay this message to another node

		uint8_t route = getChildRoute(msg.header.destination);
		if (route>0 && route<255) {
			// This message should be forwarded to a child node. If we send message
			// to this nodes pipe then all children will receive it because the are
			// all listening to this nodes pipe.
			//
			//    +----B
			//  -A
			//    +----C------D
			//
			//  We're node C, Message comes from A and has destination D
			//
			// lookup route in table and send message there
			sendWrite(route, msg, len);
		} else  {
			// A message comes from a child node and we have no
			// route for it.
			//
			//    +----B
			//  -A
			//    +----C------D    <-- Message comes from D
			//
			//     We're node C
			//
			// Message should be passed to node A (this nodes relay)

			// This message should be routed back towards sensor net gateway
			sendWrite(nc.parentNodeId, msg, len);
			// Add this child to our "routing table" if it not already exist
			addChildRoute(sender, last);
		}
	}
	return false;
}


MyMessage& MySensor::getLastMessage() {
	return msg;
}


void MySensor::saveState(uint8_t pos, uint8_t value) {
	if (loadState(pos) != value) {
		eeprom_write_byte((uint8_t*)(EEPROM_LOCAL_CONFIG_ADDRESS+pos), value);
	}
}
uint8_t MySensor::loadState(uint8_t pos) {
	return eeprom_read_byte((uint8_t*)(EEPROM_LOCAL_CONFIG_ADDRESS+pos));
}


void MySensor::addChildRoute(uint8_t childId, uint8_t route) {
	if (childNodeTable[childId] != route) {
		childNodeTable[childId] = route;
		eeprom_write_byte((uint8_t*)EEPROM_ROUTES_ADDRESS+childId, route);
	}
}

void MySensor::removeChildRoute(uint8_t childId) {
	if (childNodeTable[childId] != 0xff) {
		childNodeTable[childId] = 0xff;
		eeprom_write_byte((uint8_t*)EEPROM_ROUTES_ADDRESS+childId, 0xff);
	}
}

uint8_t MySensor::getChildRoute(uint8_t childId) {
	return childNodeTable[childId];
}


int8_t pinIntTrigger = 0;
void wakeUp()	 //place to send the interrupts
{
	pinIntTrigger = 1;
}
void wakeUp2()	 //place to send the second interrupts
{
	pinIntTrigger = 2;
}

void MySensor::internalSleep(unsigned long ms) {
	while (!pinIntTrigger && ms >= 8000) { LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF); ms -= 8000; }
	if (!pinIntTrigger && ms >= 4000)    { LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF); ms -= 4000; }
	if (!pinIntTrigger && ms >= 2000)    { LowPower.powerDown(SLEEP_2S, ADC_OFF, BOD_OFF); ms -= 2000; }
	if (!pinIntTrigger && ms >= 1000)    { LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF); ms -= 1000; }
	if (!pinIntTrigger && ms >= 500)     { LowPower.powerDown(SLEEP_500MS, ADC_OFF, BOD_OFF); ms -= 500; }
	if (!pinIntTrigger && ms >= 250)     { LowPower.powerDown(SLEEP_250MS, ADC_OFF, BOD_OFF); ms -= 250; }
	if (!pinIntTrigger && ms >= 125)     { LowPower.powerDown(SLEEP_120MS, ADC_OFF, BOD_OFF); ms -= 120; }
	if (!pinIntTrigger && ms >= 64)      { LowPower.powerDown(SLEEP_60MS, ADC_OFF, BOD_OFF); ms -= 60; }
	if (!pinIntTrigger && ms >= 32)      { LowPower.powerDown(SLEEP_30MS, ADC_OFF, BOD_OFF); ms -= 30; }
	if (!pinIntTrigger && ms >= 16)      { LowPower.powerDown(SLEEP_15Ms, ADC_OFF, BOD_OFF); ms -= 15; }
}

void MySensor::sleep(unsigned long ms) {
	// Let serial prints finish (debug, log etc)
	Serial.flush();
	driver->powerDown();
	pinIntTrigger = 0;
	internalSleep(ms);
}

bool MySensor::sleep(uint8_t interrupt, uint8_t mode, unsigned long ms) {
	// Let serial prints finish (debug, log etc)
	bool pinTriggeredWakeup = true;
	Serial.flush();
	driver->powerDown();
	attachInterrupt(interrupt, wakeUp, mode);
	if (ms>0) {
		pinIntTrigger = 0;
		sleep(ms);
		if (0 == pinIntTrigger) {
			pinTriggeredWakeup = false;
		}
	} else {
		Serial.flush();
		LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
	}
	detachInterrupt(interrupt);
	return pinTriggeredWakeup;
}

int8_t MySensor::sleep(uint8_t interrupt1, uint8_t mode1, uint8_t interrupt2, uint8_t mode2, unsigned long ms) {
	int8_t retVal = 1;
	Serial.flush(); // Let serial prints finish (debug, log etc)
	driver->powerDown();
	attachInterrupt(interrupt1, wakeUp, mode1);
	attachInterrupt(interrupt2, wakeUp2, mode2);
	if (ms>0) {
		pinIntTrigger = 0;
		sleep(ms);
		if (0 == pinIntTrigger) {
			retVal = -1;
		}
	} else {
		Serial.flush();
		LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
	}
	detachInterrupt(interrupt1);
	detachInterrupt(interrupt2);

	if (1 == pinIntTrigger) {
		retVal = (int8_t)interrupt1;
	} else if (2 == pinIntTrigger) {
		retVal = (int8_t)interrupt2;
	}
	return retVal;
}

#ifdef DEBUG
void MySensor::debugPrint(const char *fmt, ... ) {
	char fmtBuffer[300];
	if (isGateway) {
		// prepend debug message to be handled correctly by gw (C_INTERNAL, I_LOG_MESSAGE)
	//	snprintf_P(fmtBuffer, 299, PSTR("0;0;%d;0;%d;"), C_INTERNAL, I_LOG_MESSAGE);
	//	Serial.print(fmtBuffer);
	}
	va_list args;
	va_start (args, fmt );
	va_end (args);
	if (isGateway) {
		// Truncate message if this is gateway node
		vsnprintf_P(fmtBuffer, 60, fmt, args);
		fmtBuffer[59] = '\n';
		fmtBuffer[60] = '\0';
	} else {
		vsnprintf_P(fmtBuffer, 299, fmt, args);
	}
	va_end (args);
	Serial.print(fmtBuffer);
	Serial.flush();

	//Serial.write(freeRam());
}
#endif


#ifdef DEBUG
int MySensor::freeRam (void) {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
#endif
