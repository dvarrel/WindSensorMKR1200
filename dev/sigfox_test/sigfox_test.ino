/*

*/

#include <SigFox.h>

#define Serial Serial1
#define DEBUG true
// cpu clock to reduce power 2mA@1Mhz / 14mA@48Mhz
#define CPU_DIVISOR 48
#define FULL 1

typedef struct __attribute__ ((packed)) sigfox_message {
  int8_t data1;
  uint8_t data2;
  uint8_t data3;
  uint8_t lastMessageStatus;
} SigfoxMessage;

SigfoxMessage msg;
void reboot();
void sendSigFoxMessage();
void cpu_speed(int divisor);

void setup() {
  cpu_speed(FULL);
  if (DEBUG){
  if (CPU_DIVISOR != 1)
      Serial.begin(9600*CPU_DIVISOR);
    else
      Serial.begin(115200);
    uint8_t waiting=0;
    while(!Serial && waiting++<90) delay(1);
  }
  if (!SigFox.begin()) {
    // Something is really wrong, try rebooting
    // Reboot is useful if we are powering the board using an unreliable power source
    // (eg. solar panels or other energy harvesting methods)
    reboot();
  }

  String ID = SigFox.ID();
  SigFox.end();

  cpu_speed(CPU_DIVISOR);
  if (DEBUG) Serial.println(ID);
  delay(100/CPU_DIVISOR);
}


void loop(){
  if (DEBUG){
    Serial.println("START Sigfox");
    delay(100/CPU_DIVISOR);
  }
    
  uint32_t timer = millis();
  cpu_speed(FULL);
  sendSigFoxMessage();
  cpu_speed(CPU_DIVISOR);
  if (DEBUG){
    Serial.print("STOP Sigfox ");
    Serial.println(millis()-timer);
  }

  for(byte i=0;i<10;i++){
    if (DEBUG) Serial.print(i);
    delay(1000/CPU_DIVISOR);
  }
  if (DEBUG) Serial.println();
}

void sendSigFoxMessage(){
  // Start the module
  SigFox.begin();
  // Wait at least 30ms after first configuration (100ms before)
  delay(100);

  SigFox.debug();

  float temperature = SigFox.internalTemperature();
  msg.data1 = (int8_t)(temperature);

  
  // Clears all pending interrupts
  SigFox.status();
  delay(1);
  SigFox.beginPacket();
  SigFox.write((uint8_t*)&msg, 4); // 4 bytes
  msg.lastMessageStatus = SigFox.endPacket();
  
  SigFox.end();
}


void reboot() {
  NVIC_SystemReset();
  while (1);
}

void cpu_speed(int divisor){
  if (CPU_DIVISOR != 1){
    GCLK->GENDIV.reg = GCLK_GENDIV_DIV(divisor) |         // Divide the 48MHz clock source by divisor 48: 48MHz/48=1MHz
                     GCLK_GENDIV_ID(0);            // Select Generic Clock (GCLK) 0
    while (GCLK->STATUS.bit.SYNCBUSY);               // Wait for synchronization      
  }
}
