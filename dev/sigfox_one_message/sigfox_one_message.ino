#include <SigFox.h>
#define Serial Serial1

typedef struct __attribute__ ((packed)) sigfox_message {
  int8_t data1;
  uint8_t data2;
  uint8_t data3;
  uint8_t lastMessageStatus;
} SigfoxMessage;

SigfoxMessage msg;
void reboot();

void setup() {
  pinMode(  
    Serial.begin(115200);
    while(!Serial);
    // Start the module
    if (!SigFox.begin()) reboot();
    // Wait at least 30ms after first configuration (100ms before)
    Serial.println("sigfox start");
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
    Serial.println("sigfox end");
}

void loop(){
  
}

void reboot() {
  NVIC_SystemReset();
  while (1);
}
