// Copyright Benoit Blanchon 2014-2016
// MIT License
//
// Arduino JSON library
// https://github.com/bblanchon/ArduinoJson

#include <ArduinoJson.h>
#include <Time.h>
#include <TimeLib.h>


void doNecesaryActionsUponResponse(String inputJsonString) {
  if (MAIN_DEBUG) DEBUG_OUTPUT.println("  F:doNecesaryActionsUponResponse(String inputJsonString): " + (String)inputJsonString);

  if(responseText_global == "")
    return;

  DynamicJsonBuffer  jsonBuffer;

  JsonObject& root = jsonBuffer.parseObject(inputJsonString);

  // Test if parsing succeeds.
  if (!root.success()) 
  {
    showServiceMessage("NO Response!");   

    stressAskingForResponse();
    notParsedHttpResponses_errorCount++; totalErrorCount_global++;

    if(SHOW_ERROR_DEBUG_INFORMATION) DEBUG_OUTPUT.println("---->Response not parsed!");
    if(SHOW_ERROR_DEBUG_INFORMATION) DEBUG_OUTPUT.println((String)notParsedHttpResponses_errorCount + " responses not parsed, " + parsedHttpResponses_notErrorCount + " parsed OK!");
    if(INTERNET_COMMUNICATION_DEBUG) DEBUG_OUTPUT.println("NOT Parsed response Text:" + inputJsonString);

    return;
  } 
  
  parsedHttpResponses_notErrorCount++;
  showServiceMessage("Response OK!");
  resetObjectAskingForResponse();
  responseText_global = "";

  // Fetch values.
  //
  // Most of the time, you can rely on the implicit casts.
  // In other case, you can do root["time"].as<long>();
  byte topHeatingTemp = root["topHeatingTemp"];
  byte lowDropingTemp = root["lowDropingTemp"];
  bool powerOff = root["powerOff"];
  byte lastTemperature = root["lastTemperature"];
  String heatingControl = root["heatingControl"];
  String nowTime = root["nowTime"];
  String nowDate = root["nowDate" ];
  bool resetNode = root["resetNode"];
  String syncTime = root["syncTime"];


  if (INTERNET_COMMUNICATION_DEBUG) 
  {
    Serial.print("topHeatingTemp: "); Serial.println(topHeatingTemp);
    Serial.print("lowDropingTemp: "); Serial.println(lowDropingTemp);
    Serial.print("powerOff: "); Serial.println(powerOff);
    Serial.print("lastTemperature: "); Serial.println(lastTemperature);
    Serial.print("nowTime: "); Serial.println(nowTime);
    Serial.print("nowDate: "); Serial.println(nowDate);
    Serial.print("resetNode: "); Serial.println(resetNode);
  }

  //----------------Procesing----------------
  nodeStatusUpdateTime_global = root["nodeStatusUpdateTime"];

  if (heatingControl == "ARDUINO")
    setTempControleStyle(ARDUINO_STYLE_CONTROL);
  else if (heatingControl == "MANUAL")
    setTempControleStyle(MANUAL_STYLE_CONTROL);
  else if (heatingControl == "OFF")
    setTempControleStyle(OFF_STYLE_CONTROL);


  if (resetNode)
    restartEsp();

  topHeatingTemp_global = topHeatingTemp; 
  lowDropingTemp_global = lowDropingTemp;
  nowTime_global = nowTime;
  nowDate_global = nowDate;
  if(getObjectAskingForResponse().indexOf("begining state update") != -1);
    setTime(syncTime.substring(0,2).toInt(),syncTime.substring(3,5).toInt(),syncTime.substring(6,8).toInt(),syncTime.substring(9,11).toInt(),syncTime.substring(12,14).toInt(),syncTime.substring(15,19).toInt());
  resetObjectAskingForResponse();

  //digitalClockDisplay();

}

String getNowTimeDateString() 
{
  return getNowTimeString() + " " + getNowDateString();  
}

String getNowTimeString() 
{
  // digital clock display of the time
  String timeDateString = "";  
  timeDateString += hour();
  timeDateString += ":";
  if(minute() < 10) 
    timeDateString += "0";
  timeDateString += minute();
  timeDateString += ":";
  if(second() < 10)
    timeDateString += "0";
  timeDateString += second(); 

  return timeDateString;   
}

String getNowDateString() 
{
  // digital clock display of the time
  String timeDateString = "";  
  timeDateString += day();
  timeDateString += ".";
  timeDateString += month();
  timeDateString += ".";
  timeDateString += year();

  return timeDateString;   
}


void setObjectAskingForResponse(String object)
{
  if (WAITING_FOR_RESPONSES_MODULE_ENABLED)
    objectAskingForResponse_global += object;
}

bool isSomebodyAskingForResponse()
{
  return (objectAskingForResponse_global != "") ? true : false;
}

String getObjectAskingForResponse()
{
  return objectAskingForResponse_global;
}

void resetObjectAskingForResponse()
{
  objectAskingForResponse_global = "";
}

void stressAskingForResponse()
{
  objectAskingForResponse_global = "!" + objectAskingForResponse_global;
}





