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
