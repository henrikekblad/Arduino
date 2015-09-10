
// H: Ett "bas"-exempel på nod som både skickar och tar emot data


#include <MySensor.h>
#include <SPI.h>

#define DEVICE_ID 1
#define RELAY_PIN  3  // digital I/O pin where relay is attached
#define BUTTON_PIN  3  // digital I/O pin where on/off button is attached

MySensor gw;
MsgDeviceTripped tripped(DEVICE_ID);

bool state = RELAY_ON;

void setup()  
{   
  // Initialize library and add callback for incoming messages
  gw.begin(incomingMessage);

  // Register sensors to gw (they will be created as child devices)
  gw.present(DEVICE_ID, DEV_LIGHT);

  // Set relay pins in output mode and turn on relay
  pinMode(RELAY_PIN, OUTPUT);   
  digitalWrite(RELAY_PIN, state);

  // Bet button pin as input
  pinMode(BUTTON_PIN, INPUT);   

}


void loop() 
{
  // Alway process incoming messages whenever possible
  gw.process();

  // check if user wants to turn on or off relay (pushbutton) 	
  if (digitalRead(BUTTON_PIN) == HIGH) {
	state = !state;
	digitalWrite(RELAY_PIN, state);
	tripped.status = status;
    tripped.send();
    while (digitalRead(BUTTON_PIN) == HIGH) {};
  }
}

void incomingMessage(const MsgDeviceTripped &in) {
   // Message received from controller to change state
   digitalWrite(RELAY_PIN, in.status);
   status = in.status;
}

