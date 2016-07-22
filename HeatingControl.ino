//HeatingControl.ino









#define OFF_STYLE_CONTROL 0
#define ARDUINO_STYLE_CONTROL 1
#define MANUAL_STYLE_CONTROL 2



const char * const TEMP_CONTROL_STYLE[] =
{
	"OFF",
    "ARDUINO",
    "MANUAL"
};

void controlHeating_loop(float namerenaTeplotaVBojleru)
{
  if (MAIN_DEBUG) DEBUG_OUTPUT.println("  F:controlHeating_loop(namerenaTeplotaVBojleru): " + (String)namerenaTeplotaVBojleru);

  if(boilerTempControlStyle_global != ARDUINO_STYLE_CONTROL)
  {
    if(RELAY_DEBUG) DEBUG_OUTPUT.println("RELAY_DEBUG: NO heating control on: " + (String)TEMP_CONTROL_STYLE[boilerTempControlStyle_global] + ", returning..");
    return;
  }

  if (namerenaTeplotaVBojleru <= lowDropingTemp_global)
    turnOnBoilerHeating();

  if(namerenaTeplotaVBojleru >= topHeatingTemp_global)
    turnOffBoilerHeating();
}


void turnOnBoilerHeating()
{
  if(MAIN_DEBUG) DEBUG_OUTPUT.println("  F:turnOnBoilerHeating()");
  
  if(boilerTempControlStyle_global == OFF_STYLE_CONTROL)
  {
  	if(RELAY_DEBUG) DEBUG_OUTPUT.println("RELAY_DEBUG: NO heating control on: " + (String)TEMP_CONTROL_STYLE[boilerTempControlStyle_global] + ", returning..");
  	return;
  }
  else  
  {
    if(setOpenBoilerHeatingRelay(false))
    	sendNodeStateUpdate("BoilerHeating_ON");
  }
}


void turnOffBoilerHeating()
{
  if(MAIN_DEBUG) DEBUG_OUTPUT.println("  F:turnOffBoilerHeating()");
  
  if(boilerTempControlStyle_global == MANUAL_STYLE_CONTROL)
  {
  	if(RELAY_DEBUG) DEBUG_OUTPUT.println("RELAY_DEBUG: NO heating control on: " + (String)TEMP_CONTROL_STYLE[boilerTempControlStyle_global] + ", returning..");
  	return;
  }
  else
  	if(setOpenBoilerHeatingRelay(true))
	  sendNodeStateUpdate("BoilerHeating_OFF");
}


void setTempControleStyle(byte newState)  //1 - arduino(programatic), 2 - Manual(thermostat), 0 - Off eletkricity)
{
  if(MAIN_DEBUG) DEBUG_OUTPUT.println("  F:setTempControleStyle(newState): " + (String)TEMP_CONTROL_STYLE[newState]);

  if(newState != boilerTempControlStyle_global)
  {
    if(RELAY_DEBUG) DEBUG_OUTPUT.println("---->RELAY_DEBUG: Setting new style of TempControl: " + (String)TEMP_CONTROL_STYLE[newState]);
    
    boilerTempControlStyle_global = newState;    
    sendNodeStateUpdate((String)"TempControl_" + (String)TEMP_CONTROL_STYLE[newState]);
    
    if(newState == OFF_STYLE_CONTROL)
    	turnOffBoilerHeating();

	if(newState == MANUAL_STYLE_CONTROL)
		turnOnBoilerHeating();
  }
  else
    if(RELAY_DEBUG) DEBUG_OUTPUT.println("---->RELAY_DEBUG: Already set ProgramaticTempControl: " + (String)TEMP_CONTROL_STYLE[newState]);
}


void cutOffBoilerElektricity()
{
  if(MAIN_DEBUG) DEBUG_OUTPUT.println("  F:cutOffBoilerElektricity(): ");

  if(boilerTempControlStyle_global != OFF_STYLE_CONTROL)
  {
  	setTempControleStyle(OFF_STYLE_CONTROL);
  }
}


byte getTempControleStyle()
{
  return boilerTempControlStyle_global;
}


String getTempControleStyleStringName()
{
  return TEMP_CONTROL_STYLE[boilerTempControlStyle_global];
}

bool isBoilerHeatingOn()
{
	return !isBoilerHeatingRelayOpen();
}










