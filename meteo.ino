
// web: http://www.henningkarlsen.com/electronics
/*.
 Make sure all the display driver and pin connections are correct by
 editing the User_Setup.h file in the TFT_eSPI library folder.
 #########################################################################
 ###### DON'T FORGET TO UPDATE THE User_Setup.h FILE IN THE LIBRARY ######
 #########################################################################
 ------------------ Pensez a renommez le User_Setup.h
 */
#include <esp_now.h>
#include <WiFi.h>
#include "SPI.h"
#include "TFT_eSPI.h"
#include <BME280I2C.h> 
#include <Wire.h>

#include <time.h>
const char* ssid       = "DNA-Mokkula-2G-7M3EQF";
const char* password   = "47890783266";
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

float temp_ext = 0;   float t_max = temp_ext;   float t_min = 30;
long temps;

typedef struct struct_message {
    float c;
    //float d;
} struct_message;
struct_message myData;

BME280I2C bme;    // Default : forced mode, standby time = 1000 ms
                  // Oversampling = pressure ×1, temperature ×1, humidity ×1, filter off, // --------------
#define TFT_GREY 0x7BEF

TFT_eSPI myGLCD = TFT_eSPI();       // Invoke custom library

void setup()                         // ----- Début du setup ----------------
  {
//connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println(" CONNECTED");
  
  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

 
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  while(!Serial) {} // Wait
  Wire.begin();
  while(!bme.begin())
  {
    Serial.println("Could not find BME280 sensor!");
    delay(1000);
  }
 
  switch(bme.chipModel())
  {
     case BME280::ChipModel_BME280:
       Serial.println("Found BME280 sensor! Success.");
       break;
     case BME280::ChipModel_BMP280:
       Serial.println("Found BMP280 sensor! No Humidity available.");
       break;
     default:
       Serial.println("Found UNKNOWN sensor! Error!");  
  }
  
// Setup the LCD
  myGLCD.init();
  myGLCD.setRotation(0);

   // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  } 
  esp_now_register_recv_cb(OnDataRecv);
  myGLCD.fillScreen(TFT_BLACK);
  myGLCD.setTextColor(TFT_GREEN,TFT_BLACK);
  myGLCD.drawString("TEMP  IN", 10,105,4);
  //myGLCD.drawString("HEURE", 10, 100,4);
  myGLCD.drawString("PRESSION", 10, 180,4);
  myGLCD.drawString("HUMIDITE", 10, 260,4);
  myGLCD.setTextDatum(BC_DATUM); // Centre text on x,y position
  myGLCD.drawString("Bernard.picasa14@gmail.com", 160, 460,2);
  myGLCD.setTextDatum(TL_DATUM); // Remet text a default
  printLocalTime();
  temps = millis();
}                                   // ---------------- Fin du setup ------------------

void loop()                        // --------------- Début de la loop ---------
{      
  if ( (millis() - temps) > 1000*60) {
   float temp(NAN), hum(NAN), pres(NAN);
   BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
   BME280::PresUnit presUnit(BME280::PresUnit_Pa);
   bme.read(pres, temp, hum, tempUnit, presUnit);
   Serial.print("Temperature  ");Serial.print(temp);Serial.print("   Humidite  ");Serial.print(hum);Serial.print("   Pression. ");Serial.println(pres/100); //Serial.print("   Qualite AIR  ");Serial.println(mq2);
  
  myGLCD.fillScreen(TFT_BLACK);
  myGLCD.setTextColor(TFT_GREEN,TFT_BLACK);
  myGLCD.drawString("TEMP  IN", 10,105,4);
  //myGLCD.drawString("HEURE", 10, 100,4);
  myGLCD.drawString("PRESSION", 10, 180,4);
  myGLCD.drawString("HUMIDITE", 10, 260,4);
  myGLCD.setTextDatum(BC_DATUM); // Centre text on x,y position
  myGLCD.drawString("Bernard.picasa14@gmail.com", 160, 460,2);
  myGLCD.setTextDatum(TL_DATUM); 

  myGLCD.setTextColor(TFT_GREEN,TFT_BLACK);
  myGLCD.drawFloat(temp - 1.8, 1, 210, 95, 6);         //temp_in -3.7 TFT 2.8
  myGLCD.drawNumber(pres/100+17, 200, 170, 6);
  myGLCD.drawNumber(hum + 10, 220, 250, 6);
  printLocalTime();
  temps = millis() ;}       //  delay (1000*60);
}                               
// --------------- Fin de la loop -----------------

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {                                  // -------- réception données extérieur --------------
  memcpy(&myData, incomingData, sizeof(myData));
  temp_ext = myData.c;
   if (temp_ext > t_max) {t_max = temp_ext;} else if(temp_ext < t_min and t_min > -30 and temp_ext > -50) {t_min = temp_ext;}    // -------- calcul mini et maxi température extérieur ---------------
  myGLCD.setTextColor(TFT_RED,TFT_BLACK);
  myGLCD.setTextColor(TFT_ORANGE,TFT_BLACK); myGLCD.drawFloat(temp_ext, 1, 130, 330, 8);
  myGLCD.setTextColor(TFT_RED,TFT_BLACK); myGLCD.drawFloat(t_max, 1, 10, 320, 6); myGLCD.setTextColor(TFT_BLUE,TFT_BLACK); myGLCD.drawFloat(t_min, 1, 10, 390, 6);  //affiche mini maxi

}
void printLocalTime()
{
  time_t rawtime;
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
   return;
  }
  char timeStringBuff[50]; //50 chars should be enough
  strftime(timeStringBuff, sizeof(timeStringBuff), "%d:%m - %H:%M", &timeinfo);
  //print like "const char*"
  Serial.println(timeStringBuff);

  //Optional: Construct String object 
  String asString(timeStringBuff);
  myGLCD.drawString(asString, 15, 15, 6);
}