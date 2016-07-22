

//RelayControl.ino




////////////////////////////////////////////////////////
//BETTER UNDERSTANDING NAMES DEFINITIONS
////////////////////////////////////////////////////////
#define ON_RELAY_STATE true
#define OFF_RELAY_STATE false

#define LED_PIN 2
#define TXD_PIN 1
#define RXD_PIN 3
//ESP8266 - GPIO 14, 16 are working properly!! others are not: 13, 1, 3, 15 
//GPIO 0 and 2,can be used only in special cases viz: http://www.instructables.com/id/ESP8266-Using-GPIO0-GPIO2-as-inputs/

//!!GPIO 16 is left next to ch_pd pin, uncorectly signed as GPIO15 on universal white board!!!
//#define HEATING_SPIRAL_RELAY_PIN 14  //LOW turn on Heating spiral, HIGH turn off Heating spiral
#define HEATING_SWITCH_OFF_RELAY_PIN 16 //LOW turn on ProgramaticTemp, HIGH turn off ProgramaticTemp

void relayBoard_setup()
{
  if(MAIN_DEBUG) DEBUG_OUTPUT.println("  F:relayBoard_setup()");
  pinMode(HEATING_SWITCH_OFF_RELAY_PIN, OUTPUT);

  //!!!! LOW - turn on a relay, HIGH it realeses!! !!!
  digitalWrite(HEATING_SWITCH_OFF_RELAY_PIN, HIGH);//default state, not connected relay
}


bool setOpenBoilerHeatingRelay(bool newState)
{
  bool relayStateChanged = false;

  if(MAIN_DEBUG) DEBUG_OUTPUT.println("  F:setOpenBoilerHeatingRelay(newState): " + (String)newState);

  if(RELAY_DEBUG) DEBUG_OUTPUT.println("RELAY_DEBUG: BoilerHeatingRelay: " + (String)((isBoilerHeatingRelayOpen()) ? "ON_RELAY_STATE" : "OFF_RELAY_STATE"));
  if(isBoilerHeatingRelayOpen() ^ newState)
  {
    if(RELAY_DEBUG) DEBUG_OUTPUT.println("---->RELAY_DEBUG: BoilerHeatingRelay: " + (String)((isBoilerHeatingRelayOpen()) ? "ON_RELAY_STATE" : "OFF_RELAY_STATE") + " at temp: " + (String)lastTemp_global);
    digitalWrite(HEATING_SWITCH_OFF_RELAY_PIN, (newState) ? LOW : HIGH);
    relayStateChanged = true;
  }
  return relayStateChanged;
}



bool isBoilerHeatingRelayOpen()
{
  return (digitalRead(HEATING_SWITCH_OFF_RELAY_PIN)) ? false : true;
}


