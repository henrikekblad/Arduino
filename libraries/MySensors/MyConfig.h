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

#ifndef MyConfig_h
#define MyConfig_h

/***
 * Configure Sensor Network
 */
 
// pick ONE of these - other board layouts can be easily added this way
#define MYSENSORS_SENSOR
//#define MYSENSORS_SERIAL_GATEWAY
//#define MYSENSORS_ETHERNET_MQTT_GATEWAY

// Choose radio type by enabling one of the following

#define MYSENSORS_RF_NRF24 // You'll find additional RF24 specific configurarion in MyDriverNRF24.cpp
#define MYSENSORS_RF_RFM69


#ifdef MYSENSORS_RF_NRF24
#include "MyDriverNRF24.h"
typedef class MyDriverNRF24 MyDriverClass;
#endif

#ifdef MYSENSORS_RF_RFM69
// Nothing here yet
#endif


/***
 * Enable/Disable debug logging
 */
#define DEBUG

#endif
