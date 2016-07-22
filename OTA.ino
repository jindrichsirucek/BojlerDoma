#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>


void OTA_setup() 
{
  if(MAIN_DEBUG) DEBUG_OUTPUT.println("  F:OTA_setup()");

  DEBUG_OUTPUT.printf("Sketch size: %ukB\n", ESP.getSketchSize() / 1000);
  DEBUG_OUTPUT.printf("Free size: %ukB\n", ESP.getFreeSketchSpace() / 1000);

 // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    DEBUG_OUTPUT.println("Start uploading");
    sendNodeStateUpdate("OTA_uploadStarted");
  });
  ArduinoOTA.onEnd([]() {
    DEBUG_OUTPUT.println("\nEnd uploading");
    sendNodeStateUpdate("OTA_uploadFinished");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    DEBUG_OUTPUT.printf("Progress: %u%%\r", (progress / (total / 100)));
    if((progress / (total / 100)) % 10 == 0)
      showServiceMessage(((String)"Progress: " + (String)(progress / (total / 100))+ "%"));

  });
  ArduinoOTA.onError([](ota_error_t error) {
    DEBUG_OUTPUT.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR)          { DEBUG_OUTPUT.println("Auth Failed");     sendNodeStateUpdate("OTA_AuthFailed");}
    else if (error == OTA_BEGIN_ERROR)    { DEBUG_OUTPUT.println("Begin Failed");    sendNodeStateUpdate("OTA_BeginFailed");}
    else if (error == OTA_CONNECT_ERROR)  { DEBUG_OUTPUT.println("Connect Failed");  sendNodeStateUpdate("OTA_ConnectFailed");}
    else if (error == OTA_RECEIVE_ERROR)  { DEBUG_OUTPUT.println("Receive Failed");  sendNodeStateUpdate("OTA_ReceiveFailed");}
    else if (error == OTA_END_ERROR)      { DEBUG_OUTPUT.println("End Failed");      sendNodeStateUpdate("OTA_EndFailed");}
  });

  ArduinoOTA.begin();

  DEBUG_OUTPUT.println("Arduino OTA setup complete..");
  DEBUG_OUTPUT.print("Waiting for OTA upload at IP address: ");
  DEBUG_OUTPUT.println(WiFi.localIP());
}


void OTA_loop(int)
{
  if(MAIN_DEBUG && (millis()%1000 < 50)) DEBUG_OUTPUT.println("  F:OTA_loop(int)");

  ArduinoOTA.handle();
}

