// Example sketch showing how to send in OneWire temperature readings
#include <MySensor.h>  
#include <SPI.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <ArduinoJson.h>

#ifndef MYSENSORS_SENSOR
#error Please switch to MYSENSORS_SENSOR in MyConfig.h
#endif

#define ONE_WIRE_BUS 3 // Pin where dallase sensor is connected 
#define MAX_ATTACHED_DS18B20 16
unsigned long SLEEP_TIME = 30000; // Sleep time between reads (in milliseconds)
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
MySensor gw;
float lastTemperature[MAX_ATTACHED_DS18B20];
int numSensors=0;
// Initialize temperature message
MsgDeviceLevel msg;



void setup()  
{ 
  // Startup OneWire 
  sensors.begin();

  // Startup and initialize MySensors library. Set callback for incoming messages. 
  gw.begin(); 



  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("Temperature Sensor", "1.0");

  // Fetch the number of attached temperature sensors  
  numSensors = sensors.getDeviceCount();

  // Present all sensors to controller
  for (int i=0; i<numSensors && i<MAX_ATTACHED_DS18B20; i++) {   
    gw.present(i, DEV_THERMOMETER);
     
  }
}


void loop()     
{     
  // Process incoming messages (like config from server)
  gw.process(); 

  // Fetch temperatures from Dallas sensors
  sensors.requestTemperatures(); 

  // Read temperatures and send them to controller 
  for (int i=0; i<numSensors && i<MAX_ATTACHED_DS18B20; i++) {
 
    // Fetch and round temperature to one decimal
    float temperature = static_cast<float>(static_cast<int>(sensors.getTempCByIndex(i)) * 10.) / 10.;
 
    // Only send data if temperature has changed and no error
    if (lastTemperature[i] != temperature && temperature != -127.00) {
      msg.set(temperature,1);
      msg.deviceId = i;
      // Send in the new temperature
      gw.send(msg);
      lastTemperature[i]=temperature;
    }
  }
  gw.sleep(SLEEP_TIME);
}



