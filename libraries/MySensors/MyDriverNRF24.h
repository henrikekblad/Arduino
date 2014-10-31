#ifndef MyCommDriver_h
#define MyCommDriver_h

#include "MyConfig.h"
#include "MyDriver.h"
#include <stdint.h>
#include "utility/RF24.h"


#ifdef MYSENSORS_SENSOR
#define RF24_CE_PIN		   9
#define RF24_CS_PIN		   10
#define RF24_PA_LEVEL 	   RF24_PA_MAX
#endif

#ifdef MYSENSORS_SERIAL_GATEWAY
#define RF24_CE_PIN		   9
#define RF24_CS_PIN		   10
#define RF24_PA_LEVEL 	   RF24_PA_LOW
#endif

#ifdef MYSENSORS_ETHERNET_MQTT_GATEWAY
#define SOFTSPI    // W5100 Ethernet has a serious problem co-exist with radio on the SPI bus. Lets use SoftSPI for RF24.
#define RF24_CE_PIN		   5
#define RF24_CS_PIN		   6
#define RF24_PA_LEVEL 	   RF24_PA_LOW
#endif

// This needs to be included after above defines
#include "utility/RF24_config.h"



#define RF24_CHANNEL	   76             //RF channel for the sensor net, 0-127
#define RF24_DATARATE 	   RF24_250KBPS   //RF24_250KBPS for 250kbs, RF24_1MBPS for 1Mbps, or RF24_2MBPS for 2Mbps
#define RF24_BASE_RADIO_ID ((uint64_t)0xA8A8E1FC00LL) // This is also act as base value for sensor nodeId addresses. Change this (or channel) if you have more than one sensor network.

#define TO_ADDR(x) (RF24_BASE_RADIO_ID + x)

#define WRITE_PIPE ((uint8_t)0)
#define CURRENT_NODE_PIPE ((uint8_t)1)
#define BROADCAST_PIPE ((uint8_t)2)

class MyDriverNRF24 : public MyDriver
{ 
public:
	MyDriverNRF24();
	void init();
	void setAddress(uint8_t address);
	uint8_t getAddress();
	bool send(uint8_t to, const void* data, uint8_t len);
	bool available(uint8_t *to);
	uint8_t receive(void* data);
	void powerDown();
private:
	RF24 *rf24;
	uint8_t _address;
};

#endif
