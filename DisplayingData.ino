//OLED_Display
/*********************************************************************
This is an example for our Monochrome OLEDs based on SSD1306 drivers

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/category/63_98

This example is for a 128x64 size display using I2C to communicate
3 pins are required to interface (2 I2C and one reset)

Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution
*********************************************************************/

#include <SPI.h> //just for compilation - to not show errors
#include <Wire.h> //I2C communication
#include <Adafruit_GFX.h> //Base graphic library
#include <ESP_SSD1306.h> //OLED display controling library

//16x2 display Version
#include <LiquidCrystal_I2C.h>
#include "symbolsDefine.h"

#define OLED_DISPLAY_ADDRESS 0x3c
#define LCD_DISPLAY_ADDRESS 0X27

LiquidCrystal_I2C lcd(LCD_DISPLAY_ADDRESS, 16, 2); // set the lcd address to 0x27 for a 16 chars and 2 line display
ESP_SSD1306 oledDisplay(-1);// -1 means no reset pin, library edited

#define serviceAreaDisplayStartCol 0
#define serviceAreaDisplayStartRow 1
#define serviceAreaDisplayLength 8

bool lcdDisplayConnected_global = false;
bool oledDisplayConnected_global = false;

void i2cBus_setup()
{
  if(MAIN_DEBUG) DEBUG_OUTPUT.println("  F:i2cBus_setup()");
  Wire.begin(4, 5); //Wire.begin(int sda, int scl) !!! labels GPIO4 and GPIO5 on EPS8266 are swaped at old esps!!!!

  Wire.beginTransmission(LCD_DISPLAY_ADDRESS);
  if (Wire.endTransmission() == 0)
    lcd_setup();

  Wire.beginTransmission(OLED_DISPLAY_ADDRESS);
  if (Wire.endTransmission() == 0)
    oledDisplay_setup();
 }

void lcd_setup()
{ 
  if(MAIN_DEBUG) DEBUG_OUTPUT.println("  F:lcd_setup()");
  lcdDisplayConnected_global = true;
  // initialize the lcd
  lcd.begin();
  lcd.createChar(6, newCharDrop);
  lcd.createChar(7, newCharTemp);

  // Turn on the blacklight and print a message.
  lcd.backlight();
  showServiceMessage("Loading...");
  lcd.setCursor(9, 0);
  lcd.write(6);//drop symbol
  displayPrintAt("0.0L   " , 10, 0);

  lcd.setCursor(0, 0);
  lcd.write(7);//temp symbol
}


void oledDisplay_setup()
{
  if(MAIN_DEBUG) DEBUG_OUTPUT.println("  F:oledDisplay_setup()");
  oledDisplayConnected_global = true;
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  oledDisplay.begin(SSD1306_SWITCHCAPVCC, OLED_DISPLAY_ADDRESS);  // initialize with the I2C addr 0x3c (for the 128x64)
  oledDisplay.clearDisplay();
  oledDisplay.display();

  //if(!DEBUG_REMOTE_CONSOLE) remoteConsole.assignDisplayOutput(&oledDisplay, 0,2,150);
  oledDisplay.setTextSize(1);
  oledDisplay.setTextColor(WHITE, BLACK);
}


void displayData_loop(int)
{
  if(MAIN_DEBUG) DEBUG_OUTPUT.println("  F:displayData_loop()");
  
  if(waterFlowSensorCount_ISR_global > 1)
    showServiceMessage(getWaterFlowLitresInMinutes() + "");  
  else
    showServiceMessage(getStringUpTime() + "  ");

  char bufferCharConversion[10];               //temporarily holds data from vals
  int row = 0;
  //Temperature
  //  lcd.write(7);//temp symbol

  if(lcdDisplayConnected_global)
  {
    if(lastTemp_global == ERROR_TEMP_VALUE_MINUS_127)
      displayPrintAt(" -- ", 1, 0); // prints
    else
    {
      dtostrf(lastTemp_global, 2, 2, bufferCharConversion);  //float value lastTemp_global is copied onto buff bufferCharConversion
      displayPrintAt(bufferCharConversion, 1, 0); // prints temp on OLED
      lcd.print((char)223);//Degree Symbol°
  //    oledDisplay.write(759);//° symbol
      displayPrint("C");


      dtostrf(pipeTemp_global, 2, 2, bufferCharConversion);  //float value lastTemp_global is copied onto buff bufferCharConversion
      displayPrintAt(bufferCharConversion, 10, 1); // prints temp on OLED
      lcd.print((char)223);//Degree Symbol°
      //oledDisplay.write(759);//° symbol
      displayPrint("C");


    }
  }

  if(oledDisplayConnected_global)
  {
    //float namerenaTeplotaVBojleru = readTemperature();
    if(lastTemp_global != ERROR_TEMP_VALUE_MINUS_127)
      if(lastTemp_global)
      {
        //lcd.write('^');
        oledDisplay.write(24);//up arrow symbol
      }
      else
      {
        //lcd.write('_');
        oledDisplay.write(25);//down arrow symbol
      }
  }


  //If changed Water Flow state
  if(waterFlowSensorCount_ISR_global > lastWaterFlowSensorCount_global || waterFlowDisplay_global == 0)
  {
    dtostrf(convertWaterFlowSensorImpulsesToLitres(waterFlowDisplay_global + lastWaterFlowSensorCount_global), 4, 1, bufferCharConversion);  //4 is mininum width, 4 is precision; float value is copied onto buff
    displayPrintAt(bufferCharConversion, 10, row); // prints flow on OLED
    displayPrint("L  ");
  }

  if(oledDisplayConnected_global)
  {
    oledDisplay.setCursor(0, 4 * 8);//4*8 4.řádek * 8bodů font
    oledDisplay.print("MainTemp: ");
    oledDisplay.print(lastTemp_global);
    oledDisplay.println("    ");
    oledDisplay.print("FlowTemp: ");
    oledDisplay.print(flowTemp_global);
    oledDisplay.println("     ");
    oledDisplay.print("PipeTemp: ");
    oledDisplay.print(pipeTemp_global);
    oledDisplay.println("                                                  ");

    oledDisplay.display();
  }
}


void showServiceMessage(String message) {showServiceMessage(message.c_str());}
void showServiceMessage(char const* message)
{
  if(DISPLAY_DEBUG) DEBUG_OUTPUT.print("DsiplayServiceMessage: ");
  if(DISPLAY_DEBUG) DEBUG_OUTPUT.println(message);

  eraseServiceDisplayArea();
  displayPrintAt(message, 0, 1);
}


void eraseServiceDisplayArea()
{
  const char* message = "         ";      
  
  if(lcdDisplayConnected_global)
  {

    lcd.setCursor(serviceAreaDisplayStartCol, serviceAreaDisplayStartRow);
    lcd.print(message);
  }

  if(oledDisplayConnected_global)
  {
    oledDisplay.setCursor(serviceAreaDisplayStartCol * 6, serviceAreaDisplayStartRow * 8);
    oledDisplay.print(message);
    oledDisplay.display();
  }
}

/////////////////////////////////////////////////////
//////////////Internal functions/////////////////////
/////////////////////////////////////////////////////
void displayPrintAt(char const* message, int col, int row)
{
  if(lcdDisplayConnected_global)
  {
    lcd.setCursor(col, row);
    lcd.print(message);
  }

  yield();

  if(oledDisplayConnected_global)
  {
    oledDisplay.setCursor(col * 6, row * 8);
    oledDisplay.print(message);
    oledDisplay.display();
  }
}

void displayPrint(char const* message)
{
  if(lcdDisplayConnected_global)
    lcd.print(message);

  if(oledDisplayConnected_global)
    oledDisplay.print(message);
}





String getTimeFromLastUpdate()// upTime
{
  String timeFromLastTempUpdate = "";
  uint32_t milisecondsFromLastSend = (millis() - lastUpdateTime_global);

  int hoursFromLastSend = milisecondsFromLastSend / 1000 / 60 / 60;
  int minutesFromLastSend = milisecondsFromLastSend / 1000 / 60 - hoursFromLastSend * 60;
  int secondsFromLastSend = milisecondsFromLastSend / 1000 - minutesFromLastSend * 60 - hoursFromLastSend * 60 * 60;

  //char bufferCharConversion[15];
  //sprintf(bufferCharConversion, "%02d:%02d:%02d  ", hoursFromLastSend, minutesFromLastSend, secondsFromLastSend);

  timeFromLastTempUpdate = (String)hoursFromLastSend + ":" + (String)minutesFromLastSend + ":" + (String)secondsFromLastSend;
  return timeFromLastTempUpdate;
}

String getStringUpTime()
{
  long upTimeInMilliseconds = millis();

  int hoursFromLastSend = upTimeInMilliseconds / 1000 / 60 / 60;
  int minutesFromLastSend = upTimeInMilliseconds / 1000 / 60 - hoursFromLastSend * 60;
  int secondsFromLastSend = upTimeInMilliseconds / 1000 - minutesFromLastSend * 60 - hoursFromLastSend * 60 * 60;

  return (String)hoursFromLastSend + ":" + (String)minutesFromLastSend + ":" + (String)secondsFromLastSend;
}



