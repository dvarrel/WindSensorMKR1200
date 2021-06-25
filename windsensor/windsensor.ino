/*
 NB: When flashing the MKR, it may be in "deep sleep" 
 which causes a non-detection of the COM port. 
 In order to wake it up, you need to tap twice on the "RST" button
 and the board will be redetected by your PC.

https://github.com/arduino-libraries/SigFox/issues/16
 modif line 188,251,338 in Sigfox.cpp LED OFF
 */

#include <SigFox.h>
#include "def.h"

#define Led LED_BUILTIN  //(6)

static uint32_t timer;
static uint8_t tickDay;
Station station;

volatile unsigned long count;
volatile unsigned long contactBounceTime;  // Timer to avoid contact bounce in interrupt routine

void cpu_speed(int divisor);
void sendSigFoxMessage(uint8_t len);
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
  tickDay = 0;

}

void loop() {
  if(DEBUG) {
    while (Serial.available()>0){
      byte inByte = Serial.read();
      if (inByte=='S'){
        Serial.println("sending SigFox from Serial");
        cpu_speed(FULL);
        sendSigFoxMessage(8);
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
      tickDay++;
      if(DEBUG) {
        Serial.print("sending SigFox tickDay=");
        Serial.print(tickDay);Serial.print("/");
        Serial.println(TICK_DAY);
        delay(500/CPU_DIVISOR);
      }
      cpu_speed(FULL);
      sendSigFoxMessage(8);
      if (tickDay == TICK_DAY){
        sendSigFoxMessage(12);
        tickDay = 0;
      }
      cpu_speed(CPU_DIVISOR);
      if(DEBUG){
          String s = "Ubat=" +String(station.u_bat);
          s += "V N=" + String(station.N); 
          s += " \t T="+ String(station.SigfoxWindMessage.temperature);
          s += " \t P="+ String(station.SigfoxWindMessage.pressure - encodedGapPressure);
          s += " \t H="+ String(station.SigfoxWindMessage.humidity);
          s += " \t sig_error=" + String(station.SigfoxWindMessage.lastMessageStatus); 
          Serial.println(s);

          if (SigFox12bytes) s = " 12bytes = ";
          else s = " 8bytes = ";
          char buffer[64];
          for (byte i=0;i<2;i++){
            snprintf(buffer, sizeof(buffer), "%02X,%02X,%02X,%02X,",
            station.SigfoxWindMessage.speedMin[i],
            station.SigfoxWindMessage.speedAvg[i],
            station.SigfoxWindMessage.speedMax[i],
            station.SigfoxWindMessage.directionAvg[i]);
            s += String(buffer);
          }
          if (SigFox12bytes) {
            snprintf(buffer, sizeof(buffer), "%02X,%02X,%02X,%02X,",
            station.SigfoxWindMessage.batteryVoltage,
            station.SigfoxWindMessage.temperature,
            station.SigfoxWindMessage.pressure,
            station.SigfoxWindMessage.humidity);
            s += String(buffer);
          }
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


void sendSigFoxMessage(uint8_t len) {
  // Start the module
  delay(10);
  SigFox.begin();
  // Wait at least 30mS after first configuration (100mS before)
  SigFox.debug();
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
  SigFox.write((uint8_t)station.SigfoxWindMessage.speedMin[0]);
  SigFox.write((uint8_t)station.SigfoxWindMessage.speedMin[1]);
  SigFox.write((uint8_t)station.SigfoxWindMessage.speedAvg[0]);
  SigFox.write((uint8_t)station.SigfoxWindMessage.speedAvg[1]);
  SigFox.write((uint8_t)station.SigfoxWindMessage.speedMax[0]);
  SigFox.write((uint8_t)station.SigfoxWindMessage.speedMax[1]);
  SigFox.write((uint8_t)station.SigfoxWindMessage.directionAvg[0]);
  SigFox.write((uint8_t)station.SigfoxWindMessage.directionAvg[1]);
  if (len==12) {
    SigFox.write((uint8_t)station.SigfoxWindMessage.batteryVoltage);
    SigFox.write((int8_t)station.SigfoxWindMessage.temperature);
    SigFox.write((uint8_t)station.SigfoxWindMessage.pressure);
    SigFox.write((uint8_t)station.SigfoxWindMessage.humidity);
  }
  int ret = SigFox.endPacket();
  SigFox.end();
  station.SigfoxWindMessage.lastMessageStatus=ret; 
}

void isr_rotation ()   {
  noInterrupts();
  if ((micros() - contactBounceTime) > BOUNCE_TIME/CPU_DIVISOR ) {  // debounce the switch contact.
    count++;
    contactBounceTime = micros();
  }
  interrupts();
}

void reboot() {
  NVIC_SystemReset();
  while (1) ;
}
