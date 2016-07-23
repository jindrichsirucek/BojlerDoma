/*
  Created by Jindrich SIRUCEK
  See www.jindrichsirucek.cz
  Email: jindrich.sirucek@gmail.com
*/


//--------------------------------------------------------------------------------
//Libraries included
#include <ESP8266WiFi.h>
#include <Tasker.h>

#define OSTRY_PROVOZ true //if true? disable debugging output information
#define DISABLE_SOME_MODULES true // disables some modules

#define NODE_NAME "Doma"

#if DISABLE_SOME_MODULES
  #define OTA_MODULE_ENABLED true
  #define DISPLAY_MODULE_ENABLED true
  #define WATER_FLOW_MODULE_ENABLED true
  #define CURRENT_MODULE_ENABLED true
  #define TEMP_MODULE_ENABLED true
  #define WAITING_FOR_RESPONSES_MODULE_ENABLED true
  #define UPLOADING_DATA_MODULE_ENABLED true
  #define SENDING_BEACON_MODULE_ENABLED true       //TODO:Node posílá každou hodinu info o tom, že žije 


#else //All Modules enabled 
  #define OTA_MODULE_ENABLED true
  #define DISPLAY_MODULE_ENABLED true
  #define WATER_FLOW_MODULE_ENABLED true
  #define CURRENT_MODULE_ENABLED true
  #define TEMP_MODULE_ENABLED true
  #define WAITING_FOR_RESPONSES_MODULE_ENABLED true
  #define UPLOADING_DATA_MODULE_ENABLED true
  #define SENDING_BEACON_MODULE_ENABLED true       //TODO:Node posílá každou hodinu info o tom, že žije


#endif
#if OSTRY_PROVOZ //Real mode! no debuging
  #define MAIN_DEBUG true
  #define TEMPERATURE_DEBUG false
  #define DEBUG_REMOTE_CONSOLE false
  #define DISPLAY_DEBUG false
  #define RELAY_DEBUG false
  #define CURRENT_DEBUG false
  #define INTERNET_COMMUNICATION_DEBUG false
  #define SHOW_ERROR_DEBUG_INFORMATION true
  #define HTTPS_REDIRECT_DEBUG true

//RemoteConsole remoteConsole("jindrichsirucek.cz" , "/sensorData_readConsole.php", 100);//, &Serial); //server url, script url, internal buffer size

#else //Development mode! Sets what to debug
  #define NODE_NAME "development"
  #define MAIN_DEBUG true
  #define TEMPERATURE_DEBUG false
  #define DEBUG_REMOTE_CONSOLE true
  #define DISPLAY_DEBUG false
  #define RELAY_DEBUG true
  #define CURRENT_DEBUG false
  #define INTERNET_COMMUNICATION_DEBUG false
  #define SHOW_ERROR_DEBUG_INFORMATION false
  #define HTTPS_REDIRECT_DEBUG false

#endif
//RemoteConsole remoteConsole("jindrichsirucek.cz" , "/sensorData_readConsole.php", 100);//, &Serial); //server url, script url, internal buffer size



////////////////////////////////////////////////////////
//PROJECT SETTINGS
////////////////////////////////////////////////////////
//WIFI AP  
#define AP_SSID "APJindra"
#define AP_PASSWORD "cargocargo"

//#define AP_SSID "UPC3246803"
//#define AP_PASSWORD "PDFDFKXG"

#define AP_SSID "UPC3049010"
#define AP_PASSWORD "RXYDNHRD"

#define DATA_SERVER_HOST_ADDRESS "script.google.com"//Google services :-) it works now!!
#define DATA_SERVER_SCRIPT_URL "/macros/s/AKfycbw19MD0EY_ynw22Az6qgWtdC2rdz9dik5dWc-N8CH6VZvbsix6q/exec"; //script ESP8266WebServer aplication url

////////////////////////////////////////////////////////
//AUTONOMY SETTINGS
////////////////////////////////////////////////////////
#define TEMP_DIFFERENCE_TO_ASK_FOR_RESPONSE 1 //in degrees of celsia
#define MIMIMAL_SENDING_TEMP_DIFFERENCE 0.07 // after the difference between two measurments cross this level, data will be uploaded. Lower values bigger acuracy, but values can jump up and down around one value - too many samples with no real value
#define ERROR_TEMP_VALUE_MINUS_127 -127
#define ERROR_TEMP_VALUE_PLUS_85 85
////////////////////////////////////////////////////////
//SAFETY MAX and MIN SETTINGS
////////////////////////////////////////////////////////
#define CUTTING_OFF_BOILER_ELECTRICITY_SAFE_TEMPERATURE 80
#define MAX_ERROR_COUNT_PER_HOUR 70
#define TASKER_MAX_TASKS 10 // define max number of tasks to save precious Arduino RAM, 10 is default value
////////////////////////////////////////////////////////
//GLOBAL VARIABLES
////////////////////////////////////////////////////////

float lastTemp_global = ERROR_TEMP_VALUE_MINUS_127;
float flowTemp_global = 0;
float pipeTemp_global = ERROR_TEMP_VALUE_MINUS_127;
byte topHeatingTemp_global = 40;
byte lowDropingTemp_global = 35;
byte boilerTempControlStyle_global = 2; //1 - arduino(programatic), 2 - Manual(thermostat), 0 - Off eletkricity)

unsigned long lastUpdateTime_global = 0;
unsigned long nodeStatusUpdateTime_global = 60 * 60 * 1000; //60 * 60 * 1000 = 1 hour

float bigiestCurrentSample_global = 0;
unsigned int countCurrentSamples_global = 0;
float lastCurrentMeasurment_global = 0;

unsigned long lastWaterFlowSensorCount_global = 0;
volatile unsigned long waterFlowSensorCount_ISR_global;  // Measures flow meter pulses
unsigned long waterFlowDisplay_global = 0;
unsigned long lastWaterFlowResetTime_global = 0; //time from last measured water flow
unsigned long detachedTime_global;

/// Restart will be triggered on this time
unsigned long espRestartTime_global = 160 * 1000; //this value need to be changed also in function itself

String systemStateInfo_global = "";
String objectAskingForResponse_global = "begining state update";
String responseText_global = "";

String nowTime_global = "";
String nowDate_global = "";
////////////////////////////////////////////////////////
//DEBUGING VARIABLES and SETTINGS
////////////////////////////////////////////////////////
float t1 = -3;
float t2 = -4;
float td = 5;

uint16_t notParsedHttpResponses_errorCount = 0;
uint16_t parsedHttpResponses_notErrorCount = 0;
uint16_t totalErrorCount_global = 0;

#include "FS.h"

File debugFile = SPIFFS.open("/test.txt", "w");
Tasker tasker;// false - do not prioritize tasks, do not skip any task in line

#define DEBUG_OUTPUT Serial
//#define DEBUG_OUTPUT debugFile


////////////////////////////////////////////////////////

//--------------------------------------------------------------------------------

//Program itself - initialization
void setup()
{
  SPIFFS_setup();
  if (MAIN_DEBUG) DEBUG_OUTPUT.println("  F:MAIN_setup()");

  DEBUG_OUTPUT.println(""); //just to start printing on new line..  
  DEBUG_OUTPUT.print(NODE_NAME);
  DEBUG_OUTPUT.println(" node initializing..");

  if (WATER_FLOW_MODULE_ENABLED) waterFlowSensor_setup();
  if (TEMP_MODULE_ENABLED) temperature_setup();

  if (DISPLAY_MODULE_ENABLED) i2cBus_setup();
  
  relayBoard_setup();
  wifiConnect();
  yield();
  doNecesaryActionsUponResponse(responseText_global);

  if (OTA_MODULE_ENABLED) OTA_setup();

  debugFile.close();
  //debugFile = SPIFFS.open("/test.txt", "a");
}


void loop()
{
  if (MAIN_DEBUG) DEBUG_OUTPUT.println("\nF:MAIN_loop()");

  if (UPLOADING_DATA_MODULE_ENABLED) tasker.setInterval(response_loop, 10 * 1000);  
  if (SENDING_BEACON_MODULE_ENABLED) tasker.setTimeout(checkSystemState_loop, nodeStatusUpdateTime_global); //60 * 60 * 1000 = 1 hour
  if (OTA_MODULE_ENABLED)            tasker.setInterval(OTA_loop, 1000);  
  if (TEMP_MODULE_ENABLED)           tasker.setInterval(temperature_loop, 3 * 1000);
  if (DISPLAY_MODULE_ENABLED)        tasker.setInterval(displayData_loop, 1000);
  if (WATER_FLOW_MODULE_ENABLED)     tasker.setInterval(waterFlow_loop, 20 * 1000);
  if (CURRENT_MODULE_ENABLED)        tasker.setInterval(current_loop, 16 * 1000);
  
  


  //tasker.setInterval(printDebugFile_loop, 20 * 1000); // 60 * 60 * 1000 = 1 hour

  yield();
  tasker.run(); // never returns
}

//--------------------------------------------------------------------------------


void response_loop(int)
{
  delay(200);
  doNecesaryActionsUponResponse(responseText_global);
}





//Při vytváření nového logovacího souboru (nový měsíc) vyřešit to, aby se nekopírovaly hodnoty z defaultního souboru, ale z posledního souboru

//Ukládat poslední hodnoty teploty na ohřev do SPIFFS a při startu načíst (kvůli offline práci)
//No temp measurment - relay driving Off, 
//zamezit zbytečném relé spínání
//Upravit erase display area.. zdržuje to kod.. přidat mezery na konec slova do plného počtu display arre
//poslat zprávu že přijal změnu refresh time nodeinfoupdate

//ukládat info během sprchy do souboru a vůbec neposílat
//ukládat to ve formě JSONu
//Pak to poslat celé naráz až se vypne voda
//pokud existuje soubor tak ho pošli s timestamps, přestat používat příchozí timestamps, ale programatické, z vnitřního časovače





void temperature_loop(int)
{
  if (MAIN_DEBUG) DEBUG_OUTPUT.println("  F:temperature_loop()");
  float namerenaTeplotaVBojleru = readTemperature(1);
  if (MAIN_DEBUG) DEBUG_OUTPUT.println("  F:readTemperature() returned: " + (String)namerenaTeplotaVBojleru);

  if (namerenaTeplotaVBojleru == ERROR_TEMP_VALUE_MINUS_127 || namerenaTeplotaVBojleru == 85)
  {
    totalErrorCount_global++;
    lastTemp_global = ERROR_TEMP_VALUE_MINUS_127;
    return;
  }

  if(lastTemp_global == ERROR_TEMP_VALUE_MINUS_127)
  {
    lastTemp_global = namerenaTeplotaVBojleru;
    return;//minimálně dvě měření po sobě jdoucí musí být skutečné hodnoty
  }

  if (abs(namerenaTeplotaVBojleru - lastTemp_global) >= MIMIMAL_SENDING_TEMP_DIFFERENCE)
  {
    if (abs((int)namerenaTeplotaVBojleru - (int)lastTemp_global) >= 1) //pokud měření právě překročilo hranici stupně
      if (((int)namerenaTeplotaVBojleru % TEMP_DIFFERENCE_TO_ASK_FOR_RESPONSE) == 0) //každé dva stupně si zažádej o vedení
        setObjectAskingForResponse("Temp_" + (String)TEMP_DIFFERENCE_TO_ASK_FOR_RESPONSE);
    lastTemp_global = namerenaTeplotaVBojleru;
    DEBUG_OUTPUT.print("Temperature has changed.. sending update data: ");
    DEBUG_OUTPUT.println(lastTemp_global);

    sendNodeStateUpdate("Temp_changed");
  }

  if (namerenaTeplotaVBojleru >= CUTTING_OFF_BOILER_ELECTRICITY_SAFE_TEMPERATURE)
  {
    cutOffBoilerElektricity();
    sendNodeStateUpdate(String("BOILER_TEMP_OverHeating_" + (String)namerenaTeplotaVBojleru).c_str());
  }
  
  controlHeating_loop(namerenaTeplotaVBojleru);
}




void current_loop(int)
{
  if (MAIN_DEBUG) DEBUG_OUTPUT.println("  F:current_loop()");
    
  int currentNow = getMaxCurrentValue();
  if (abs(lastCurrentMeasurment_global - currentNow) > 1000)
  {
    if (lastCurrentMeasurment_global > 1000 && currentNow < 200) //Pokud se právě vypnul ohřev vody v bojleru
    {
      waterFlowDisplay_global = 0; //Vynuluj měření spotřeby teplé vody - bojler je po vypnutí ohřevu celý nahřátý
      //TODO : převést na funkci a funkci dát do display souboru kam patří
      sendNodeStateUpdate("WaterFlowDisplay_reset");
    }
    
    int lastCurrent = lastCurrentMeasurment_global;
    
    lastCurrentMeasurment_global = currentNow;
    sendNodeStateUpdate((lastCurrent > currentNow) ? "Current_dropped" : "Current_rised");
  }
  else
    lastCurrentMeasurment_global = currentNow;
}


void waterFlow_loop(int)
{
  if (MAIN_DEBUG) DEBUG_OUTPUT.println("  F:waterFlow_loop()");

  if (readFlowInLitres() < 2 && lastWaterFlowSensorCount_global > 10)//watter stopped flowing
  {
    DEBUG_OUTPUT.println("Flow state has droped to zero.. sending update data..");
    sendNodeStateUpdate("WaterFlow_Stopped");

    //waterFlowDisplay_global = 0; //zresetuj počítadlo spotřeoavné vody na displayi
  }
  if (readFlowInLitres() > 0 && lastWaterFlowSensorCount_global <= 10) //watter started to flow
    lastWaterFlowResetTime_global = millis();
}


String getSystemStateInfo()
{

  String systemStateInfo = "";
  //if ((totalErrorCount_global / millis() / 1000 / 60 / 60) >= MAX_ERROR_COUNT_PER_HOUR)
    //ESP.restart();
  systemStateInfo += "Uptime: " + getStringUpTime() + "\n";
  systemStateInfo += "\nnodeStatusUpdateTime_global: " + formatTimeToString(nodeStatusUpdateTime_global);
  systemStateInfo += "\ntopHeatingTemp_global: " + (String)topHeatingTemp_global;
  systemStateInfo += "\nlowDropingTemp_global: " + (String)lowDropingTemp_global;

  //systemStateInfo +=\n "TimeFromLastUpdate: " + getTimeFromLastUpdate() + "\n";
  systemStateInfo += "\ntotalErrorCount_global: " + (String)totalErrorCount_global + "\n";
  //systemStateInfo += "\nERROR_COUNT_PER_HOUR: " + (String)(totalErrorCount_global / millis() / 1000 / 60 / 60) + "\n";

  systemStateInfo += "\nnotParsedHttpResponses_errorCount: " + (String)notParsedHttpResponses_errorCount + "\n";
  systemStateInfo += "\nparsedHttpResponses_notErrorCount: " + (String)parsedHttpResponses_notErrorCount + "\n";

  systemStateInfo += "\nlastUpdateTime_global: " + formatTimeToString(lastUpdateTime_global);
  systemStateInfo += "\nlastWaterFlowSensorCount_global: " + (String)lastWaterFlowSensorCount_global;
  systemStateInfo += "\nwaterFlowDisplay_global: " + (String)waterFlowDisplay_global;
  systemStateInfo += "\nlastWaterFlowResetTime_global: " + formatTimeToString(lastWaterFlowResetTime_global);

  systemStateInfo += "\nTime&Date: " + getNowTimeDateString();

  return systemStateInfo;
}


void checkSystemState_loop(int)
{
  setObjectAskingForResponse("checkSystemState_loop");
  String systemStateInfo = "\n\n";

  if (MAIN_DEBUG) DEBUG_OUTPUT.println("  F:checkSystemState_loop()");

  systemStateInfo += getESPStatusUpdate();

  sendNodeStateUpdate("Beacon_Alive");//zároveň slouží jako beacon alive
  
  systemStateInfo_global = "";

  //if (SHOW_ERROR_DEBUG_INFORMATION) DEBUG_OUTPUT.println(systemStateInfo);
  //if (SHOW_ERROR_DEBUG_INFORMATION) WiFi.printDiag(Serial);  

  if (SENDING_BEACON_MODULE_ENABLED) 
    tasker.setTimeout(checkSystemState_loop, nodeStatusUpdateTime_global); //60 * 60 * 1000 = 1 hour
}


void restartEsp()
{
    sendNodeStateUpdate("ESP_restart");
    ESP.restart(); 
}
//--------------------------------------------------------------------------------

void wifiConnect()
{
  if (MAIN_DEBUG) DEBUG_OUTPUT.println("  F:wifiConnect()");
  showServiceMessage("WiFi");

  WiFi.mode(WIFI_STA);
  WiFi.begin(AP_SSID, AP_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    DEBUG_OUTPUT.print(".");
    displayPrint(".");
  }
  DEBUG_OUTPUT.println("");
  if (INTERNET_COMMUNICATION_DEBUG) { DEBUG_OUTPUT.print("WiFi connected to: "); DEBUG_OUTPUT.println(AP_SSID);}

  sendNodeStateUpdate("WiFi_connected");
}





