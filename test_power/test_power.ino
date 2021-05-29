/*
 * measure power in different state
  */
#include <SigFox.h>
#include <ArduinoLowPower.h>

#define Led   LED_BUILTIN  //(6)
#define cpuDivisor 48
#define full 1

#define Serial Serial1

void setup() {
  cpu_speed(full);
  Serial.begin(115200);
  delay(10);
  Serial.println("setup");
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

void cpu_speed(int divisor){
  GCLK->GENDIV.reg = GCLK_GENDIV_DIV(divisor) |         // Divide the 48MHz clock source by divisor 48: 48MHz/48=1MHz
                   GCLK_GENDIV_ID(0);            // Select Generic Clock (GCLK) 0
  while (GCLK->STATUS.bit.SYNCBUSY);               // Wait for synchronization      
}

void crystalSpeed(){
  GCLK->GENCTRL.reg = GCLK_GENCTRL_IDC |           // Set the duty cycle to 50/50 HIGH/LOW
                      GCLK_GENCTRL_GENEN |         // Enable GCLK0
                      GCLK_GENCTRL_SRC_XOSC32K |   // Set the external 32.768kHz clock source (XOSC32K)
                      GCLK_GENCTRL_ID(0);          // Select GCLK0
  while (GCLK->STATUS.bit.SYNCBUSY);               // Wait for synchronization  
}

void dfll48Speed(){
  GCLK->GENCTRL.reg = GCLK_GENCTRL_IDC |           // Set the duty cycle to 50/50 HIGH/LOW
                      GCLK_GENCTRL_GENEN |         // Enable GCLK0
                      GCLK_GENCTRL_SRC_DFLL48M |   // Set the external 32.768kHz clock source (XOSC32K)
                      GCLK_GENCTRL_ID(0);          // Select GCLK0
  while (GCLK->STATUS.bit.SYNCBUSY);               // Wait for synchronization  
}



void loop()
{
  cpu_speed(cpuDivisor);
  Serial.println("LED ON 5s");
  digitalWrite(Led,1); //17mA
  delay(5000/cpuDivisor);
  Serial.println("LED OFF 5s");
  digitalWrite(Led,0);
  delay(5000/cpuDivisor);

  cpu_speed(full);  
  String s= "123456789012";
  Serial.println("SendSigFox "+ s);
  sendString(s);

  Serial.println("deepSleep 10s");
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
