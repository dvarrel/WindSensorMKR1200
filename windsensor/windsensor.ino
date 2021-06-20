/*
 NB: When flashing the MKR, it may be in "deep sleep" 
 which causes a non-detection of the COM port. 
 In order to wake it up, you need to tap twice on the "RST" button
 and the board will be redetected by your PC.

 modif line 188 & 251 in Sigfox.cpp LED OFF
 */

#include <SigFox.h>
#include "def.h"

#define Led LED_BUILTIN  //(6)

static uint32_t timer;
Station station;

volatile unsigned long count;
volatile unsigned long ContactBounceTime;  // Timer to avoid contact bounce in interrupt routine


void cpu_speed(int divisor);
void sendSigFoxMessage();
void isr_rotation();
void reboot();


void setup() {
  cpu_speed(FULL);
  pinMode(pinGirAlim,OUTPUT);
  pinMode(pinAnemo,INPUT_PULLUP);
  pinMode(Led, OUTPUT);
  pinMode(pinBme280Vcc,OUTPUT);
  pinMode(pinBme280Gnd,OUTPUT);

  digitalWrite(pinBme280Vcc,HIGH);
  digitalWrite(pinBme280Gnd,LOW);

  attachInterrupt(digitalPinToInterrupt(pinAnemo), isr_rotation, FALLING);
  analogReadResolution(adcResolutionBits);

  if(DEBUG) {
    if (CPU_DIVISOR != 1)
      Serial.begin(9600*CPU_DIVISOR);
    else
      Serial.begin(115200);
    uint8_t waiting=0;
    while(!Serial && waiting++<90) delay(1);
    Serial.println("Setup...");
  }
  
  station.init(DEBUG);
  
  if (!SigFox.begin()) {
    if(DEBUG) Serial.println("Something wrong with SigFox module, rebooting...");
    reboot();
  }
  if(DEBUG) {
    String version = SigFox.SigVersion();
    String ID = SigFox.ID();
    String PAC = SigFox.PAC();
    // Display module informations
    Serial.println("SigFox FW version " + version);
    Serial.println("ID  = " + ID);
    Serial.println("PAC = " + PAC);
  }
  // Send the module to the deepest sleep
  SigFox.end();

  cpu_speed(CPU_DIVISOR);
  timer = millis();

}

void loop() {
  if(DEBUG) {
    while (Serial.available()>0){
      byte inByte = Serial.read();
      if (inByte=='S'){
        Serial.println("sending SigFox from Serial");
        cpu_speed(FULL);
        sendSigFoxMessage();
        cpu_speed(CPU_DIVISOR);
      }
      else Serial.write(inByte);
    }
  }

  uint32_t deltaT = millis() - timer;
  if ( deltaT > TICK_DELAY ) {
    uint16_t c = count;
    count = 0 ;
    timer = millis();
    station.add_measure(c, deltaT);
      
    if (DEBUG) station.print();
    if(station.tick==0xFF){
      if(DEBUG) {
        Serial.println("sending SigFox");
        delay(500/CPU_DIVISOR);
      }
      cpu_speed(FULL);
      sendSigFoxMessage();
      cpu_speed(CPU_DIVISOR);
      if(DEBUG){
          String s = "Ubat=" +String(station.u_bat);
          s += "V N=" + String(station.N); 
          s += " \t T="+ String(station.SigfoxWindMessage.temperature);
          s += " \t P="+ String(station.SigfoxWindMessage.pressure - encodedDeltaPressure);
          s += " \t H="+ String(station.SigfoxWindMessage.humidity);
          s += " \t sig_error=" + String(station.SigfoxWindMessage.lastMessageStatus); 
          Serial.println(s);

          s = " 12bytes = ";
          for (byte i=0;i<2;i++){
            s += String(station.SigfoxWindMessage.speedMin[i], HEX);
            s += String(station.SigfoxWindMessage.speedAvg[i], HEX);
            s += String(station.SigfoxWindMessage.speedMax[i], HEX);
            s += String(station.SigfoxWindMessage.directionAvg[i], HEX);
          }
          s += String(station.SigfoxWindMessage.batteryVoltage, HEX);
          s += String(station.SigfoxWindMessage.temperature, HEX);
          s += String(station.SigfoxWindMessage.pressure, HEX);
          s += String(station.SigfoxWindMessage.humidity, HEX);
          Serial.println(s);
        }
    }

  }
}

void cpu_speed(int divisor){
  if (CPU_DIVISOR != 1){
    GCLK->GENDIV.reg = GCLK_GENDIV_DIV(divisor) |         // Divide the 48MHz clock source by divisor 48: 48MHz/48=1MHz
                     GCLK_GENDIV_ID(0);            // Select Generic Clock (GCLK) 0
    while (GCLK->STATUS.bit.SYNCBUSY);               // Wait for synchronization      
  }
}


void sendSigFoxMessage() {
  // Start the module
  delay(10);
  SigFox.begin();
  // Wait at least 30mS after first configuration (100mS before)
  SigFox.debug();  // no LED
  delay(100);
  station.batteryVoltage();
  // add last error to humidity byte -> error=0 humidity even else odd
  if (station.SigfoxWindMessage.lastMessageStatus==0)
    station.SigfoxWindMessage.humidity &= 0xFE;
  else
    station.SigfoxWindMessage.humidity |= 0x01;
  // Clears all pending interrupts
  SigFox.status();
  delay(1);
  SigFox.beginPacket();
  for (byte i=0;i<2;i++){
    SigFox.write((uint8_t)station.SigfoxWindMessage.speedMin[i]);
    SigFox.write((uint8_t)station.SigfoxWindMessage.speedAvg[i]);
    SigFox.write((uint8_t)station.SigfoxWindMessage.speedMax[i]);
    SigFox.write((uint8_t)station.SigfoxWindMessage.directionAvg[i]);
  }
  SigFox.write((uint8_t)station.SigfoxWindMessage.batteryVoltage);
  SigFox.write((int8_t)station.SigfoxWindMessage.temperature);
  SigFox.write((uint8_t)station.SigfoxWindMessage.pressure);
  SigFox.write((uint8_t)station.SigfoxWindMessage.humidity);
  int ret = SigFox.endPacket();
  SigFox.end();
  station.SigfoxWindMessage.lastMessageStatus=ret;  
}

void isr_rotation ()   {
  noInterrupts();
  if ((micros() - ContactBounceTime) > 10000/CPU_DIVISOR ) {  // debounce the switch contact.
    count++;
    ContactBounceTime = micros();
  }
  interrupts();
}

void reboot() {
  NVIC_SystemReset();
  while (1) ;
}
