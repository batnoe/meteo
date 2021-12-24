
#include <DallasTemperature.h>
#include <OneWire.h>
OneWire oneWire(13);                             //Bus One Wire sur la pin 2 de l'arduino
DallasTemperature sensors(&oneWire);            //Utilistion du bus Onewire pour les capteurs
DeviceAddress sensorDeviceAddress;              //Vérifie la compatibilité des capteurs avec la librairie

#include <ESP8266WiFi.h>  //ESP8266
#include <espnow.h>       //ESP8266

uint8_t broadcastAddress1[] = {0x9C, 0x9c, 0x1F, 0xC2, 0x8A, 0x78};   // <-Upesy _{0x24, 0x6F, 0x28, 0xB1, 0x45, 0x48}; <-grand écran salle
uint8_t broadcastAddress2[] = {0x24, 0x6F, 0x28, 0xB1, 0x45, 0x48};  // TFT2.8 DEVKIT

typedef struct struct_message {
  float c;
  //float d;
} struct_message;

struct_message myData;

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  char macStr[18];
  Serial.print("Packet to:");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
         mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(macStr);
  Serial.print(" send status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
  }
}
 
void setup() {
  Serial.begin(115200);
  sensors.begin();
 
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
// Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  esp_now_add_peer(broadcastAddress1, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
  esp_now_add_peer(broadcastAddress2, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
  }
 
void loop() {
  sensors.requestTemperatures();
  myData.c = sensors.getTempCByIndex(0);
  
  // Send message via ESP-NOW
    esp_now_send(0, (uint8_t *) &myData.c, sizeof(myData.c));
  delay(1000*2);
}
