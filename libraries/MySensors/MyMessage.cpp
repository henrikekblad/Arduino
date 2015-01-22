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

#include "MyMessage.h"
#include <stdio.h>
#include <stdlib.h>

_MyTypeInfo *_MyTypeInfo::m_types = NULL;


// Getters for payload converted to desired form
void* MySensorDynamicPayload::getCustom() const {
	return (void *)data;
}

const char* MySensorDynamicPayload::getString() const {
	if (ptype == P_STRING) {
		return data;
	} else {
		return NULL;
	}
}

// handles single character hex (0 - 15)
char MySensorDynamicPayload::i2h(uint8_t i) const {
	uint8_t k = i & 0x0F;
	if (k <= 9)
		return '0' + k;
	else
		return 'A' + k - 10;
}

char* MySensorDynamicPayload::getStream(char *buffer) const {
	/*uint8_t cmd = miGetCommand();
	if ((cmd == C_STREAM) && (buffer != NULL)) {
		for (uint8_t i = 0; i < miGetLength(); i++)
		{
			buffer[i * 2] = i2h(data[i] >> 4);
			buffer[(i * 2) + 1] = i2h(data[i]);
		}
		buffer[miGetLength() * 2] = '\0'; */
		return buffer;
	/*} else {
		return NULL;
	}*/
}

char* MySensorDynamicPayload::getString(char *buffer) const {
	if (ptype == P_STRING) {
		strncpy(buffer, data, getLength());
		buffer[getLength()] = 0;
		return buffer;
	} else if (buffer != NULL) {
		if (ptype == P_BYTE) {
			itoa(bValue, buffer, 10);
		} else if (ptype == P_INT16) {
			itoa(iValue, buffer, 10);
		} else if (ptype == P_UINT16) {
			utoa(uiValue, buffer, 10);
		} else if (ptype == P_LONG32) {
			ltoa(lValue, buffer, 10);
		} else if (ptype == P_ULONG32) {
		
			ultoa(ulValue, buffer, 10);
		} else if (ptype == P_FLOAT32) {
			dtostrf(fValue,2,fPrecision,buffer);
		} else if (ptype == P_CUSTOM) {
			return getStream(buffer);
		}
		return buffer;
	} else {
		return NULL;
	}
}

uint8_t MySensorDynamicPayload::getByte() const {
	if (ptype == P_BYTE) {
		return data[0];
	} else if (ptype == P_STRING) {
		return atoi(data);
	} else {
		return 0;
	}
}

bool MySensorDynamicPayload::getBool() const {
	return getInt();
}

float MySensorDynamicPayload::getFloat() const {
	if (ptype == P_FLOAT32) {
		return fValue;
	} else if (ptype == P_STRING) {
		return atof(data);
	} else {
		return 0;
	}
}

long MySensorDynamicPayload::getLong() const {
	if (ptype == P_LONG32) {
		return lValue;
	} else if (ptype == P_STRING) {
		return atol(data);
	} else {
		return 0;
	}
}

unsigned long MySensorDynamicPayload::getULong() const {
	if (ptype == P_ULONG32) {
		return ulValue;
	} else if (ptype == P_STRING) {
		return atol(data);
	} else {
		return 0;
	}
}

int MySensorDynamicPayload::getInt() const {
	if (ptype == P_INT16) {
		return iValue;
	} else if (ptype == P_STRING) {
		return atoi(data);
	} else {
		return 0;
	}
}

unsigned int MySensorDynamicPayload::getUInt() const {
	if (ptype == P_UINT16) {
		return uiValue;
	} else if (ptype == P_STRING) {
		return atoi(data);
	} else {
		return 0;
	}

}




// Set dynamic payload
MyPayload& MySensorDynamicPayload::set(void* value, uint8_t _length) {
	ptype = P_CUSTOM;
	setLength(_length);
	memcpy(data, value, min(_length, MAX_PAYLOAD_SIZE));
	return *this;
}


MyPayload& MySensorDynamicPayload::set(const char* value) {
	setLength(strlen(value));
	ptype = P_STRING;
	strncpy(data, value, min(getLength(), MAX_PAYLOAD_SIZE));
	return *this;
}

MyPayload& MySensorDynamicPayload::set(uint8_t value) {
	setLength(1);
	ptype = P_BYTE;
	data[0] = value;
	return *this;
}


MyPayload& MySensorDynamicPayload::set(float value, uint8_t decimals) {
	ptype = P_FLOAT32;
	setLength(5); // 32 bit float + persi
	fValue=value;
	fPrecision = decimals;
	return *this;
}

MyPayload& MySensorDynamicPayload::set(unsigned long value) {
	ptype = P_ULONG32;
	setLength(4);
	ulValue = value;
	return *this;
}

MyPayload& MySensorDynamicPayload::set(long value) {
	ptype = P_LONG32;
	setLength(4);
	lValue = value;
	return *this;
}

MyPayload& MySensorDynamicPayload::set(unsigned int value) {
	ptype = P_UINT16;
	setLength(2);
	uiValue = value;
	return *this;
}

MyPayload& MySensorDynamicPayload::set(int value) {
	ptype = P_INT16;
	setLength(2);
	iValue = value;
	return *this;
}


