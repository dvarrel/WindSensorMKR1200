/*
 NB: When flashing the MKR, it may be in "deep sleep" 
 which causes a non-detection of the COM port. 
 In order to wake it up, you need to tap twice on the "RST" button
 and the board will be redetected by your PC.
 */

#include <SigFox.h>
#include <ArduinoLowPower.h>
#include "def.h"

#define Led       LED_BUILTIN  //(6)

static uint32_t timer;
Station station;

volatile unsigned long count;
volatile unsigned long ContactBounceTime;  // Timer to avoid contact bounce in interrupt routine


//#define Serial Serial1

void setup() {
  pinMode(pinGirAlim,OUTPUT);
  pinMode(pinAnemo,INPUT_PULLUP);
  pinMode(Led, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(pinAnemo), isr_rotation, FALLING);
  analogReadResolution(adcResolutionBits);
  timer = millis();

  Serial.begin(115200);
  uint8_t waiting=0;
  while(!Serial && waiting<9) {
    waiting++;
    delay(1);
  }

  station.init(true);
  
  if (!SigFox.begin()) {
    Serial.println("Something wrong with SigFox module, rebooting...");
    reboot();
  }
  // Enable debug led and disable automatic deep sleep
  // Comment this line when shipping your project :)
  //SigFox.debug();

  String version = SigFox.SigVersion();
  String ID = SigFox.ID();
  String PAC = SigFox.PAC();
  // Display module informations
  Serial.println("SigFox FW version " + version);
  Serial.println("ID  = " + ID);
  Serial.println("PAC = " + PAC);
  // Send the module to the deepest sleep
  SigFox.end();

  
}

void loop() {
  while (Serial.available()>0){
    byte inByte = Serial.read();
    if (inByte=='S'){
      Serial.println("sending SigFox");
      sendSigFoxMessage();
    }
    else Serial.write(inByte);
  }

  uint32_t deltaT = millis() - timer;
  if ( deltaT > TICK_DELAY ) {
    uint16_t c = count;
    count = 0 ;
    timer = millis();
    station.add_measure(c, deltaT);
    
    station.print();
    if(station.tick==0xFF){
      Serial.println("sending SigFox");
      sendSigFoxMessage();
    }

  }

  digitalWrite(Led,0);
  delay(1);  
  //LowPower.sleep(2000);
  delay(1);  
  delay(1000);
  digitalWrite(Led,1);

}


void sendSigFoxMessage() {
  // Start the module
  SigFox.begin();

  // Wait at least 30mS after first configuration (100mS before)
  SigFox.debug();
  delay(100);

  station.batteryVoltage();
  Serial.print("Vbat=");Serial.println(station.u_bat);

  // Clears all pending interrupts
  //SigFox.status();
  //delay(1);
  SigFox.beginPacket();
  for (byte i=0;i<2;i++){
    SigFox.write((uint8_t)station.SigfoxWindMessage.speedMin[i]);
    SigFox.write((uint8_t)station.SigfoxWindMessage.speedAvg[i]);
    SigFox.write((uint8_t)station.SigfoxWindMessage.speedMax[i]);
    SigFox.write((uint8_t)station.SigfoxWindMessage.directionAvg[i]);
  }
    SigFox.write((uint8_t)station.SigfoxWindMessage.batteryVoltage);
    SigFox.write((uint8_t)station.SigfoxWindMessage.lastMessageStatus);
  //SigFox.write((uint8_t*)&SigfoxWindMessage,sizeof(SigfoxWindMessage));
  int ret = SigFox.endPacket();
  station.SigfoxWindMessage.lastMessageStatus=ret;
  Serial.print("code erreur=");Serial.println(ret);
  
  SigFox.end();
}

void isr_rotation ()   {
  noInterrupts();
  if ((millis() - ContactBounceTime) > 10 ) {  // debounce the switch contact.
    count++;
    ContactBounceTime = millis();
  }
  interrupts();
}

void reboot() {
        NVIC_SystemReset();
        while (1) ;
}
