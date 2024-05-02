#include <WiFi.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "f1UdpStruct.h"
#include "lwip/sockets.h"


#define MAX_BUFFER_SIZE 1500

#define MotionID 0
#define SessionID 1
#define LapDataID 2
#define EventID 3
#define ParticipantsID 4
#define CarSetupsID 5
#define CarTelemetryID 6
#define CarStatusID 7
#define FinalClassificationID 8
#define LobbyInfoID 9 
#define CarDamageID 10
#define SessionHistoryID 11
#define TyreSetsID 12
#define MotionExID 13


const char* ssid = "";
const char* password = "";

const char* target_ip = "192.168.1.15";
int port = 20777;

WiFiUDP udp;
struct sockaddr_in client_address;

TFT_eSPI tft = TFT_eSPI();

int gear = 9;
int SessionType=-1;
int revLightBefore = 0;
int prevErsPrc = -1;

const int leds[] = {1,2,42,41,40,39,38,37,36};
const int NumberOfLeds =6;




void setup() {

  for(int i = 0 ; i < NumberOfLeds ; i++){
    pinMode(leds[i], OUTPUT);
    digitalWrite(leds[i], LOW);
  }

  tft.begin();
  tft.setRotation(3);  
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  
  
  ConnectToWiFi();
  setDisplay();
 
  udp.begin(port);

}

void loop() {
  
  
  
  int client_address_len = sizeof(client_address);

  char buffer[MAX_BUFFER_SIZE];
  int recv_size = udp.parsePacket();

  if (recv_size > 0) {
    udp.read(buffer, recv_size);

    // Ensure the received data is at least as large as the header
    if (recv_size < sizeof(struct PacketHeader)) {
      
      tft.fillScreen(TFT_BLACK);
      tft.println("Received data is too small to contain a header");
      delay(2000);
      setDisplay();
      return;
    }
  }

  // Extract the header from the received data
  struct PacketHeader* header = (struct PacketHeader*)buffer;
  
  // Now, based on the packet ID, decide the type of packet
  switch (header->m_packetId) {
    case SessionID:{
      
      uint8_t sessionType;
      size_t sessionTypeSize = sizeof(uint8_t);
      memcpy(&sessionType, buffer + sizeof(struct PacketHeader) + sizeof(uint8_t) * 2 + sizeof(uint16_t) + sizeof(int8_t) * 2, sessionTypeSize);
      if(sessionType != SessionType){
        setDisplay();
        SessionType=sessionType;
      }

      break;
    }
    case LapDataID: {
      // Handle LapData packet
      struct PacketLapData packetData;
      memcpy(&packetData.m_header, header, sizeof(struct PacketHeader));

      size_t LapDataSize = sizeof(struct LapData);
      memcpy(&packetData.m_lapData, buffer + sizeof(struct PacketHeader), LapDataSize * 22);

      size_t uintField = sizeof(uint8_t);
      memcpy(&packetData.m_timeTrialPBCarIdx, buffer + sizeof(struct PacketHeader) + LapDataSize * 22, uintField);
      memcpy(&packetData.m_timeTrialRivalCarIdx, buffer + sizeof(struct PacketHeader) + LapDataSize * 22 + uintField, uintField);

      uint32_t lapTimeMs = packetData.m_lapData[packetData.m_header.m_playerCarIndex].m_currentLapTimeInMS;
      uint32_t deltaMs = packetData.m_lapData[packetData.m_header.m_playerCarIndex].m_deltaToCarInFrontInMS;

      char lapTimeBuffer[20];
      sprintf(lapTimeBuffer, "%d:%02d.%03d", lapTimeMs / (60 * 1000), (lapTimeMs / 1000) % 60, lapTimeMs % 1000);

      char lapNumBuffer[5];
      sprintf(lapNumBuffer, "L%02d", packetData.m_lapData[packetData.m_header.m_playerCarIndex].m_currentLapNum);

      char pos[3];
      sprintf(pos, "P%02d", packetData.m_lapData[packetData.m_header.m_playerCarIndex].m_carPosition);

      char delta[15];
      sprintf(delta,"+%02d.%03d", (deltaMs / 1000) % 60, deltaMs % 1000);

      tft.drawString(delta, 193, 47, 2);
      tft.drawString(lapTimeBuffer, 141, 2, 4);
      tft.setTextColor(TFT_CYAN,TFT_BLACK);

      tft.drawString(lapNumBuffer, 13, 101, 4);
      tft.drawString(pos, 380, 101, 4);
      tft.setTextColor(TFT_WHITE,TFT_BLACK);
      break;
    }

    case CarTelemetryID: {
      // Handle PacketCarTelemetryData
      struct PacketCarTelemetryData packetData;
      memcpy(&packetData.m_header, header, sizeof(struct PacketHeader));

      size_t telemetryDataSize = sizeof(struct CarTelemetryData);
      memcpy(&packetData.m_carTelemetryData, buffer + sizeof(struct PacketHeader), telemetryDataSize * 22);

      size_t uintField = sizeof(uint8_t);
      memcpy(&packetData.m_mfdPanelIndex, buffer + sizeof(struct PacketHeader) + telemetryDataSize * 12, uintField);
      memcpy(&packetData.m_mfdPanelIndexSecondaryPlayer, buffer + sizeof(struct PacketHeader) + telemetryDataSize * 12 + uintField, uintField);

      size_t intField = sizeof(int8_t);
      memcpy(&packetData.m_mfdPanelIndexSecondaryPlayer, buffer + sizeof(struct PacketHeader) + telemetryDataSize * 12 + uintField * 2, intField);
    

      char tyre[6];

      sprintf(tyre, "%03d`C", packetData.m_carTelemetryData[packetData.m_header.m_playerCarIndex].m_tyresInnerTemperature[2]);
      tft.drawString(tyre, 18, 175, 2);
     
      sprintf(tyre, "%03d`C", packetData.m_carTelemetryData[packetData.m_header.m_playerCarIndex].m_tyresInnerTemperature[0]);
      tft.drawString(tyre, 18, 205, 2);
 

      sprintf(tyre, "%03d`C", packetData.m_carTelemetryData[packetData.m_header.m_playerCarIndex].m_tyresInnerTemperature[3]);
      tft.drawString(tyre, 385, 175, 2);
      

      sprintf(tyre, "%03d`C", packetData.m_carTelemetryData[packetData.m_header.m_playerCarIndex].m_tyresInnerTemperature[1]);
      tft.drawString(tyre, 385, 205, 2);
     

      char Speed[5];
      sprintf(Speed, "%03d", packetData.m_carTelemetryData[packetData.m_header.m_playerCarIndex].m_speed);
      
      tft.drawString(Speed, 8, 2, 4);
      
      
      char Gear[5];
      if(gear ==packetData.m_carTelemetryData[packetData.m_header.m_playerCarIndex].m_gear){
        break;
      }
      else if( packetData.m_carTelemetryData[packetData.m_header.m_playerCarIndex].m_gear == -1){
        tft.fillRect(193, 85, 97, 36, TFT_BLACK);
        tft.drawString("R", 205, 117, 1);
        gear =packetData.m_carTelemetryData[packetData.m_header.m_playerCarIndex].m_gear;
      }
      else if(packetData.m_carTelemetryData[packetData.m_header.m_playerCarIndex].m_gear == 0){
        tft.fillRect(193, 85, 97, 36, TFT_BLACK);
        tft.drawString("N", 207, 117, 1);
        gear =packetData.m_carTelemetryData[packetData.m_header.m_playerCarIndex].m_gear;
      }
      else{
        sprintf(Gear, "%d", packetData.m_carTelemetryData[packetData.m_header.m_playerCarIndex].m_gear);
        tft.drawString(Gear, 207, 112, 7);
        gear =packetData.m_carTelemetryData[packetData.m_header.m_playerCarIndex].m_gear;
      }

      
      
      if(packetData.m_carTelemetryData[packetData.m_header.m_playerCarIndex].m_drs == 0){
        tft.fillRect(188, 205, 100, 35, TFT_BLACK);
        tft.drawString("        ", 192, 207, 2);
      }
      else{
        tft.drawRect(188, 205, 100, 35, TFT_YELLOW);
        tft.drawString("DRS ON", 192, 207, 2);
      }

      
  
     
      
      
    
      
      int revLightAfter = (int)NumberOfLeds*(packetData.m_carTelemetryData[packetData.m_header.m_playerCarIndex].m_revLightsPercent*0.01);
      revLight(revLightAfter);
      revLightBefore = revLightAfter;
    
      break;
    }

    case CarStatusID: {
      struct PacketCarStatusData packetData;
      memcpy(&packetData.m_header, header, sizeof(struct PacketHeader));

      size_t carStatusSize = sizeof(struct CarStatusData);
      memcpy(&packetData.m_carStatusData, buffer + sizeof(struct PacketHeader), carStatusSize * 22);

      char fuelIn[6];
      
      
      tft.drawFloat(packetData.m_carStatusData[packetData.m_header.m_playerCarIndex].m_fuelInTank,1, 388, 6, 2);

      char fuelLeft[15];
    
      if(packetData.m_carStatusData[packetData.m_header.m_playerCarIndex].m_fuelRemainingLaps > 0){
        tft.setTextColor(TFT_GREEN,TFT_BLACK);
        sprintf(fuelLeft, "(+%.2f)", packetData.m_carStatusData[packetData.m_header.m_playerCarIndex].m_fuelRemainingLaps);
      }else{
        tft.setTextColor(TFT_RED,TFT_BLACK);
        sprintf(fuelLeft, "(%.2f)", packetData.m_carStatusData[packetData.m_header.m_playerCarIndex].m_fuelRemainingLaps);
      }
      tft.drawString(fuelLeft, 374, 47, 2);
      tft.setTextColor(TFT_WHITE,TFT_BLACK);
     

      if(packetData.m_carStatusData[packetData.m_header.m_playerCarIndex].m_ersDeployMode==3){
        
        tft.drawString("ERS  ON ", 190, 246, 2);
      }else{
       
        tft.drawString("ERS OFF", 190, 246, 2);
      }
     
      
      if(packetData.m_carStatusData[packetData.m_header.m_playerCarIndex].m_vehicleFiaFlags == 1){
        tft.fillRect(0, 243, 113, 77, TFT_GREEN);
        tft.fillRect(367, 243, 113, 77, TFT_GREEN);
      }else if(packetData.m_carStatusData[packetData.m_header.m_playerCarIndex].m_vehicleFiaFlags == 2){
        tft.fillRect(0, 243, 113, 77, TFT_BLUE);
        tft.fillRect(367, 243, 113, 77, TFT_BLUE);
      }else if(packetData.m_carStatusData[packetData.m_header.m_playerCarIndex].m_vehicleFiaFlags == 3){
        tft.fillRect(0, 243, 113, 77, TFT_YELLOW);
        tft.fillRect(367, 243, 113, 77, TFT_YELLOW);
      }else{
        tft.fillRect(0, 243, 113, 77, TFT_BLACK);
        tft.fillRect(367, 243, 113, 77, TFT_BLACK);
      }
      char ers[5];
      int ersPrc = (int)(packetData.m_carStatusData[packetData.m_header.m_playerCarIndex].m_ersStoreEnergy/4000000 * 100);
      if(prevErsPrc!=ersPrc){
        tft.fillRect(114+252*0.01*ersPrc, 283, 252*(1-(0.01*ersPrc)), 36, TFT_BLACK);
        tft.fillRect(114, 283, 252*0.01*ersPrc, 36, TFT_GREEN);
        sprintf(ers, "%*3d%%", ersPrc);
        tft.drawString(ers, 210, 285, 2);
        prevErsPrc=ersPrc;
      }

      
      break;
    }
    case CarDamageID: {
      struct PacketCarDamageData packetData;
      memcpy(&packetData.m_header, header, sizeof(struct PacketHeader));

      size_t carDamageData = sizeof(struct CarDamageData);
      memcpy(&packetData.m_carDamageData, buffer + sizeof(struct PacketHeader), carDamageData * 22);

      if(packetData.m_carDamageData[packetData.m_header.m_playerCarIndex].m_tyresWear[2]>60){
        tft.fillRoundRect(132, 91, 34, 48,4, TFT_RED);
      }else if(packetData.m_carDamageData[packetData.m_header.m_playerCarIndex].m_tyresWear[2]>40){
        tft.fillRoundRect(132, 91, 34, 48, 4,TFT_ORANGE);
      }else if(packetData.m_carDamageData[packetData.m_header.m_playerCarIndex].m_tyresWear[2]>20){
        tft.fillRoundRect(132, 91, 34, 48, 4,TFT_YELLOW);
      }else{
        tft.fillRoundRect(132, 91, 34, 48,4, TFT_GREEN);
      }

      if(packetData.m_carDamageData[packetData.m_header.m_playerCarIndex].m_tyresWear[3]>60){
        tft.fillRoundRect(315, 91, 34, 48,4, TFT_RED);
      }else if(packetData.m_carDamageData[packetData.m_header.m_playerCarIndex].m_tyresWear[3]>40){
        tft.fillRoundRect(315, 91, 34, 48, 4,TFT_ORANGE);
      }else if(packetData.m_carDamageData[packetData.m_header.m_playerCarIndex].m_tyresWear[3]>20){
        tft.fillRoundRect(315, 91, 34, 48, 4,TFT_YELLOW);
      }else{
        tft.fillRoundRect(315, 91, 34, 48,4, TFT_GREEN);
      }

      if(packetData.m_carDamageData[packetData.m_header.m_playerCarIndex].m_tyresWear[0]>60){
        tft.fillRoundRect(132, 181, 34, 48,4, TFT_RED);
      }else if(packetData.m_carDamageData[packetData.m_header.m_playerCarIndex].m_tyresWear[0]>40){
        tft.fillRoundRect(132, 181, 34, 48,4, TFT_ORANGE);
      }else if(packetData.m_carDamageData[packetData.m_header.m_playerCarIndex].m_tyresWear[0]>20){
        tft.fillRoundRect(132, 181, 34, 48,4, TFT_YELLOW);
      }
      else{
        tft.fillRoundRect(132, 181, 34, 48,4, TFT_GREEN);
      }

      if(packetData.m_carDamageData[packetData.m_header.m_playerCarIndex].m_tyresWear[1]>60){
        tft.fillRoundRect(315, 181, 34, 48,4, TFT_RED);
      }else if(packetData.m_carDamageData[packetData.m_header.m_playerCarIndex].m_tyresWear[1]>40){
        tft.fillRoundRect(315, 181, 34, 48,4, TFT_ORANGE);
      }else if(packetData.m_carDamageData[packetData.m_header.m_playerCarIndex].m_tyresWear[1]>20){
        tft.fillRoundRect(315, 181, 34, 48,4, TFT_YELLOW);
      }else{
        tft.fillRoundRect(315, 181, 34, 48,4, TFT_GREEN);
      }
      
      
      break;
    }
  }


}

void ConnectToWiFi(){
  WiFi.begin(ssid, password);

  tft.setCursor(150, 120);
  
  tft.print("Connecting to WiFi");
  int i=0;
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  
    if(i<3){
      tft.print(".");
      i++;
    }else{
      i=0;
      tft.setCursor(150, 120);
      tft.fillScreen(TFT_BLACK);
      tft.print("Connecting to WiFi");
    }
  }
  
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(150, 120);
  tft.println("Connected to WiFi");
  tft.setCursor(150, 140);
  tft.println("IP Address : ");
  tft.setCursor(150, 160);
  tft.println(WiFi.localIP());
  delay(3000);
}

void setDisplay(){
  tft.fillScreen(TFT_BLACK);
  setGrig();
  setFillRoundRect();
  setPlaceHolderData();
}

void setGrig(){
  tft.drawRect(0, 0, 113, 81, TFT_WHITE);
  tft.drawRect(113, 0, 254, 81, TFT_WHITE);
  tft.drawRect(367, 0, 113, 81, TFT_WHITE);
  tft.drawRect(0, 81, 113, 81, TFT_WHITE);
  tft.drawRect(113, 81, 254, 162, TFT_WHITE);
  tft.drawRect(367, 81, 113, 81, TFT_WHITE);
  tft.drawRect(0, 162, 113, 81, TFT_WHITE);
  tft.drawRect(367, 162, 113, 81, TFT_WHITE);
  tft.drawRect(113, 243, 254, 39, TFT_WHITE);
  tft.drawRect(113, 282, 254, 38, TFT_WHITE);
  tft.drawRect(0, 243, 113, 77, TFT_WHITE);
  tft.drawRect(367, 243, 113, 77, TFT_WHITE);
  
}

void setFillRoundRect(){
  tft.fillRoundRect(132, 91, 34, 48,4, TFT_GREEN);
  tft.fillRoundRect(315, 91, 34, 48,4, TFT_GREEN);
  tft.fillRoundRect(132, 181, 34, 48,4, TFT_GREEN);
  tft.fillRoundRect(315, 181, 34, 48,4, TFT_GREEN);
  
}

void setPlaceHolderData(){
  tft.setFreeFont(&FreeSerifBold24pt7b);
  tft.drawString("KPH", 28, 47, 2);
  tft.drawString("+00.000", 193, 47, 2);
  tft.drawString("(+0.00)", 374, 47, 2);
  tft.drawString("000", 8, 2, 4);
  tft.drawString("0:00.000", 141, 2, 4);
  tft.drawFloat(0,1, 388, 6, 2);
  tft.setTextColor(TFT_CYAN,TFT_BLACK);
  tft.drawString("L00", 13, 101, 4);
  tft.drawString("P00", 380, 101, 4);
  tft.setTextColor(TFT_WHITE,TFT_BLACK);
  tft.drawString("000`C", 18, 175, 2);
  tft.drawString("000`C", 18, 205, 2);
  tft.drawString("000`C", 385, 175, 2);
  tft.drawString("000`C", 385, 205, 2);
  tft.drawString("ERS OFF", 190, 246, 2);
  tft.drawString("  0%", 210, 285, 2);
  tft.drawString("N", 207, 115, 1);
 
}
void revLight(int revLightAfter){
  //       NumberOfLeds         ->        0
  if (revLightBefore > revLightAfter){
    for(int i = revLightBefore ; i >= revLightAfter ; i--){
      digitalWrite(leds[i], LOW);
    }
  }
  //       0        ->          NumberOfLeds
  else if(revLightBefore < revLightAfter){
    for(int i = revLightBefore ; i < revLightAfter ; i++){
      digitalWrite(leds[i], HIGH);
    }
  }
}
