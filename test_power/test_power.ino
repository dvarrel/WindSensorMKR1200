/*
 * measure power in different state
  */
#include <SigFox.h>
#include <ArduinoLowPower.h>

#define Led   LED_BUILTIN  //(6)

void setup() {
  pinMode(Led, OUTPUT);
  digitalWrite(Led,1);
  delay(100);
  digitalWrite(Led,0);
  delay(100);
 
  if (!SigFox.begin()) {
    reboot();
  }
  // Send the module to the deepest sleep
  SigFox.end();
}

void loop()
{
  digitalWrite(Led,1); //17mA
  delay(5000);
  digitalWrite(Led,0);
  
  delay(5000);

    
  sendString(String("123456789012"));

  LowPower.deepSleep(10000);

  reboot();// deepSleep works just onetime
  //the second time it use 10mA
  // so we have to reboot
  
}

void sendString(String str) {
  SigFox.begin();
  // Wait at least 30mS after first configuration (100mS before)
  delay(100);
  // Clears all pending interrupts
  SigFox.status();
  delay(1);
  SigFox.beginPacket();
  SigFox.print(str);
  SigFox.endPacket();  // send buffer to SIGFOX network

  SigFox.end();
}

void reboot() {
        NVIC_SystemReset();
        while (1) ;
}
