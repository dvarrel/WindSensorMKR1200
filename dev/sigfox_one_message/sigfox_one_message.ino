#include <SigFox.h>
//use Serial1 if no usb
#define Serial Serial1
#define led LED_BUILTIN

typedef struct __attribute__ ((packed)) sigfox_wind_message {
        uint8_t speedMin[2];
        uint8_t speedAvg[2];
        uint8_t speedMax[2];
        uint8_t directionAvg[2];
} SigfoxWindMessage_t;

SigfoxWindMessage_t msg;
void reboot();

void setup() {
    pinMode(led,OUTPUT);
    Serial.begin(115200);
    uint8_t waiting=0;
    while(!Serial && waiting++<90){
      digitalWrite(led,HIGH);
      delay(100);
      digitalWrite(led,LOW);
      delay(100);
    }
    //07060c0e181c0000
    msg.speedMin[0] = 0x07;
    msg.speedMin[1] = 0x06;
    msg.speedAvg[0] = 0x0c;
    msg.speedAvg[1] = 0x0e;
    msg.speedMax[0] = 0x18;
    msg.speedMax[1] = 0xc0;
    msg.directionAvg[0] = 0x00;
    msg.directionAvg[1] = 0x00;


}

void loop(){
    static uint16_t i = 0;
    // Start the module
    if (!SigFox.begin()) reboot();
    Serial.println(SigFox.ID());
    Serial.print("sigfox start ");
    Serial.println(i++);
    // Wait at least 30ms after first configuration (100ms before)
    delay(100);
    SigFox.debug();
   
    msg.directionAvg[0]  += 1;
    // Clears all pending interrupts
    SigFox.status();
    delay(1);
    digitalWrite(led,HIGH);
    SigFox.beginPacket();
    SigFox.write((uint8_t*)&msg, 8); // 8 bytes
    SigFox.endPacket();
    SigFox.end();
    Serial.println("sigfox end");
    digitalWrite(led,LOW);

    delay(60000);
}

void reboot() {
  NVIC_SystemReset();
  while (1);
}
