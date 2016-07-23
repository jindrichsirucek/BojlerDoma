//internet_comunication.ino
#include <HTTPSRedirect.h>




void sendNodeStateUpdate(String fireEventName){sendNodeStateUpdate(fireEventName.c_str());}
void sendNodeStateUpdate(const char* fireEventName)
{
  if (!UPLOADING_DATA_MODULE_ENABLED)
    return;

  if(MAIN_DEBUG) DEBUG_OUTPUT.println("  F:sendNodeStateUpdate(String fireEventName) :" + (String)fireEventName);
  showServiceMessage(fireEventName);

  flowTemp_global = waterFlowDisplay_global; //Debugging ##
  
  //prepare data to send
  String mainTemp = (lastTemp_global != ERROR_TEMP_VALUE_MINUS_127) ? String(lastTemp_global) : "";
  String flowTemp = (flowTemp_global != ERROR_TEMP_VALUE_MINUS_127) ? String(flowTemp_global) : "";
  String pipeTemp = (pipeTemp_global != ERROR_TEMP_VALUE_MINUS_127) ? String(pipeTemp_global) : "";

  String uriParams = "&temp=" + mainTemp + "&flowTemp=" + flowTemp + "&pipeTemp=" + pipeTemp;


  String waterFlow = "";
  if(waterFlowSensorCount_ISR_global != 0)
  {
    waterFlow = String(convertWaterFlowSensorImpulsesToLitres(waterFlowSensorCount_ISR_global));
    resetflowCount();
  }

  String current = (lastCurrentMeasurment_global != 0) ? String(lastCurrentMeasurment_global) : "";
  uriParams += "&waterFlow=" + waterFlow + "&event=" + fireEventName + "&current=" + current; // + "&" + "tGlobal" + "=" + t2 + "&" + "tDiff" + "=" + td ;

  String heatingControl = getTempControleStyleStringName();
  String heatingRelay = (isBoilerHeatingOn()) ? "ON" : "OFF";
  
  uriParams += "&heatingControl_RelayState=" + heatingControl + "&heatingSpiral_RelayState=" + heatingRelay;


  //append node identification
  char buffer[50];
  IPAddress ip = WiFi.localIP();
  sprintf(buffer, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

  uriParams = "chipId=" + String(ESP.getChipId()) + "&ip=" + URLEncode(buffer) + "&nodeName=" + URLEncode(NODE_NAME) + uriParams;
  uriParams += "&objectAskingForResponse=" + URLEncode(getObjectAskingForResponse().c_str()) + "&timeStamp=" + URLEncode(getNowTimeDateString().c_str());

  if(!UPLOADING_DATA_MODULE_ENABLED) { DEBUG_OUTPUT.println("Not sending data over WiFi: UPLOADING_DATA_MODULE: DISABLED"); return;  }

  String postDataToSent = getSystemStateInfo() + ((systemStateInfo_global != "") ? "systemStateInfo=" + systemStateInfo_global : "");    
  
  showServiceMessage("Sending");
  bool responseOk = postDataToServer(postDataToSent, uriParams);
  
  showServiceMessage("Data sent!");

  flowTemp_global++;
  lastUpdateTime_global = millis();
}


////////////////////////////////////////////////////////
//POST call from VIKTOR
////////////////////////////////////////////////////////
bool postDataToServer(String postDataToSent, String params)
{
  if(MAIN_DEBUG) DEBUG_OUTPUT.println("  F:postDataToServer(String postDataToSent, String params) : \npostDataToSent: \n" + postDataToSent + "\nparams: \n" + params);

  const String host = DATA_SERVER_HOST_ADDRESS;
  //String url = (String)"/macros/s/AKfycbzXFTXj3rW4Pu2mhSL8t-LgXHQETRlJ7tneod7cGjQu45nvlWc-/exec" + "?" + params; //viktor text file
  String url = DATA_SERVER_SCRIPT_URL;
  url +=  + "?" + params;

  HTTPSRedirect client;

  if(INTERNET_COMMUNICATION_DEBUG) DEBUG_OUTPUT.println((String)"Sending request: " + host + url);
  if(INTERNET_COMMUNICATION_DEBUG) DEBUG_OUTPUT.println((String)"Adding POST Data: " + postDataToSent);

                                      
                                      //disableInterupts();


  if (!client.POST(host, url, postDataToSent)) 
  {
      DEBUG_OUTPUT.println("Sending POST error occured");
      client.stop();
      return false;  
  }
  else if(isSomebodyAskingForResponse())
  {
    responseText_global = client.readStringUntil('\n');

    if(INTERNET_COMMUNICATION_DEBUG) DEBUG_OUTPUT.println((String)"Response text: " + responseText_global);
    //delay(100);
  }
  client.stop();


                                      //enableInterupts();


  return true;
}
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////


String URLEncode(const char* msg)
{
  const char *hex = "0123456789abcdef";
  String encodedMsg = "";

  while(*msg != '\0') {
    if( ('a' <= *msg && *msg <= 'z')
         || ('A' <= *msg && *msg <= 'Z')
         || ('0' <= *msg && *msg <= '9') ) {
      encodedMsg += *msg;
    } else {
      encodedMsg += '%';
      encodedMsg += hex[*msg >> 4];
      encodedMsg += hex[*msg & 15];
    }
    msg++;
  }
  return encodedMsg;
}
