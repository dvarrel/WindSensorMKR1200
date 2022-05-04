/*
 NB: When flashing the MKR, it may be in slow CPU mode 
 which causes a non-detection of the USB COM port. 
 In order to wake it up, you need to tap twice on the "RST" button
 and the board will be redetected by your PC.

https://github.com/arduino-libraries/SigFox/issues/16
LED OFF during transmission :
modif lines 192-194,259-261,346-348 in libraries/Arduino_SigFox_for_MKRFox1200/Sigfox.cpp 
Arduino SAMD boards version 1.8.11
 */

#include <SigFox.h>
#include "def.h"

static uint32_t timer, Ttimer;
static uint16_t nb_sigfox_day = 0;
Station station;

volatile unsigned long count;
volatile unsigned long contactBounceTime;  // Timer to avoid contact bounce in interrupt routine

void set_cpu_speed(int divisor);
uint32_t sendSigFoxMessage(uint8_t len);
void isr_rotation();
void reboot();

void setup() {
  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN,HIGH);
  set_cpu_speed(CPU_DIVISOR);
  pinMode(pinGirAlim,OUTPUT);
  pinMode(pinAnemo,INPUT);
  
  #if BMx280
  pinMode(pinBmx280Vcc,OUTPUT);
  pinMode(pinBmx280Gnd,OUTPUT);
  digitalWrite(pinBmx280Vcc,HIGH);
  digitalWrite(pinBmx280Gnd,LOW);
  #endif

  attachInterrupt(digitalPinToInterrupt(pinAnemo), isr_rotation, RISING);
  analogReadResolution(adcResolutionBits);
  
  if(DEBUG) {
    if (CPU_DIVISOR > 1)
      Serial.begin(9600*CPU_DIVISOR);
    else
      Serial.begin(115200);
    uint8_t waiting=0;
    while(!Serial && waiting++<90) delayMicroseconds(10000/CPU_DIVISOR);
    Serial.println("Setup...");
  }
  delayMicroseconds(100000/CPU_DIVISOR);
  station.init(DEBUG);

  set_cpu_speed(CPU_FULL);
  delayMicroseconds(100000/CPU_DIVISOR);
  if (!SigFox.begin()) {
    if(DEBUG) {
      set_cpu_speed(CPU_DIVISOR);
      Serial.println("Something wrong with SigFox module, rebooting...");
      Serial.flush();
    }
    reboot();
  }
  
  if(DEBUG) {
    Serial.println("SigFox FW version " + SigFox.SigVersion());
    Serial.println("ID  = " + SigFox.ID());
    Serial.println("PAC = " + SigFox.PAC());
  }
  station.set_extra_infos();
  sendSigFoxMessage(12);

  set_cpu_speed(CPU_DIVISOR);
  delayMicroseconds(10000/CPU_DIVISOR);
  if(DEBUG) {
    
  }
  delayMicroseconds(10000/CPU_DIVISOR);
  timer = millis();
  Ttimer = timer;
  count=0;

  digitalWrite(LED_BUILTIN,LOW);

}

void loop() {
  if(DEBUG) {
    while (Serial.available()>0){
      byte inByte = Serial.read();
      if (inByte=='S' || inByte=='F'){
        Serial.println("sending SigFox from Serial");
        Serial.flush();
        if (inByte=='S') sendSigFoxMessage(8);
        else sendSigFoxMessage(12);
      }
      else Serial.write(inByte);
    }
  }

  uint32_t TdeltaT = (millis() - Ttimer) * CPU_DIVISOR;
  if (TdeltaT >= MS_10MIN){ //time to send measures over sigfox network
    if(DEBUG){ 
      Serial.print("TdeltaT1=");Serial.print(TdeltaT);
      Serial.print("  ");Serial.print(nb_sigfox_day);
      Serial.print("/");Serial.println(NB_SIGFOX_DAY);
      Serial.flush();
    }
    station.compute_measures(1); //compute vmoy and gmoy and encode all
    uint32_t chrono = sendSigFoxMessage(8);
    Ttimer = millis() - (chrono/CPU_DIVISOR);
    if ( ++nb_sigfox_day >= NB_SIGFOX_DAY){ // one message per day with extra infos
      nb_sigfox_day = 0;
      uint32_t chrono = sendSigFoxMessage(12);
      Ttimer = millis() - (2*chrono/CPU_DIVISOR);
    }
    noInterrupts();
    timer = millis();
    count = 0 ;
    interrupts();
  }
  else if (TdeltaT >= MS_5MIN && station.is_first_compute() ){
    if(DEBUG){ 
      Serial.print("TdeltaT0=");Serial.println(TdeltaT);Serial.flush();
    }
    station.compute_measures(0);
  }

  uint32_t deltaT = (millis() - timer) * CPU_DIVISOR;
  if ( deltaT >= MS_3S ) {
    uint16_t c = count;
    noInterrupts();
    timer = millis();
    count = 0 ;
    interrupts();
    
    station.add_measure(c, deltaT);
  }
}

void set_cpu_speed(int divisor){
  if (CPU_DIVISOR != 1){
    GCLK->GENDIV.reg = GCLK_GENDIV_DIV(divisor) |         // Divide the 48MHz clock source by divisor 48: 48MHz/48=1MHz
                     GCLK_GENDIV_ID(0);            // Select Generic Clock (GCLK) 0
    while (GCLK->STATUS.bit.SYNCBUSY);               // Wait for synchronization      
  }
}

uint32_t sendSigFoxMessage(uint8_t len) {
  uint32_t chrono = millis();
  set_cpu_speed(CPU_FULL);
  delay(10);
  SigFox.begin();
  delay(100);
  SigFox.debug();
   station.set_extra_infos();
  // Clears all pending interrupts
  SigFox.status();
  delay(1);
  SigFox.beginPacket();
  SigFox.write((uint8_t*)&station.SigfoxWindMsg, len);
  station.SigfoxWindMsg.lastMessageStatus = SigFox.endPacket();
  SigFox.end();
  set_cpu_speed(CPU_DIVISOR);
  uint32_t r = millis();
  if(DEBUG){
    station.print_extra_infos();
    station.print_sigfox_msg(len);
  }
  return (r - chrono);
}

void isr_rotation ()   {
  //noInterrupts();
  if ((micros() - contactBounceTime) > BOUNCE_TIME/CPU_DIVISOR ) {  // debounce the switch contact.
    count++;
    contactBounceTime = micros();
  }
  //interrupts();
}

void reboot() {
  NVIC_SystemReset();
  while (1) ;
}
