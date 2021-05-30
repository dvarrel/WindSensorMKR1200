/*
 * https://forum.arduino.cc/t/isr-timer-capture-on-samd21/685456/11
 * Count the number of pulses on pin D10 (PA19) 
 * MKRFOX 1200 pins D10 -> PA19
 */
#include <ArduinoLowPower.h>

//#define Serial Serial1

void setup()
{
  Serial.begin(115200);                         // Initialise the native serial port
  while(!Serial); // Wait for the console to open
  Serial.println("Setup...");
  PM->APBCMASK.reg |= PM_APBCMASK_EVSYS;           // Switch on the event system peripheral

  ////////////////////////////////////////////////////////////////////////////////////////
  // Generic Clock Initialisation 
  ////////////////////////////////////////////////////////////////////////////////////////

  GCLK->GENDIV.reg =  GCLK_GENDIV_DIV(1) |          // Select clock divisor to 1                     
                      GCLK_GENDIV_ID(4);            // Select GLCK4         

  GCLK->GENCTRL.reg = GCLK_GENCTRL_IDC |            // Set the duty cycle to 50/50 HIGH/LOW 
                      GCLK_GENCTRL_GENEN |          // Enable GCLK                   
                      GCLK_GENCTRL_SRC_XOSC32K |    // Select GCLK source as external 32.768kHz crystal (XOSC32K)                          
                      GCLK_GENCTRL_ID(4);           // Select GCLK4             
  while (GCLK->STATUS.bit.SYNCBUSY);                // Wait for synchronization

 /* GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN |          // Enable generic clock
                      GCLK_CLKCTRL_GEN_GCLK0 |      // GCLK0 at 48MHz 
                      GCLK_CLKCTRL_ID_TCC2_TC3;      // As a clock source for TC4 and TC5
*/ 
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN |          // Enable generic clock
                      GCLK_CLKCTRL_GEN_GCLK4 |      // GCLK4 at 32.768kHz
                      GCLK_CLKCTRL_ID_TC4_TC5;     // As a clock source for TCC2 and TC3

  ////////////////////////////////////////////////////////////////////////////////////////
  // TC4 Initialisation - measurement counter: counts the number incoming of pulses
  ////////////////////////////////////////////////////////////////////////////////////////

  PORT->Group[PORTA].PINCFG[19].bit.PMUXEN = 1;                            // Enable the port multiplexer on port pin PA19 (D10)
  PORT->Group[PORTA].PMUX[19 >> 1].reg |= PORT_PMUX_PMUXO_A;               // Set-up PA19 (D10) as an EIC (interrupt)
  
  EIC->EVCTRL.reg |= EIC_EVCTRL_EXTINTEO3;                                 // Enable event output on external interrupt 3
  EIC->CONFIG[0].reg |= EIC_CONFIG_SENSE3_HIGH;                            // Set event detecting a HIGH level
  EIC->INTENCLR.reg = EIC_INTENCLR_EXTINT3;                                // Clear the interrupt flag on channel 3
  EIC->CTRL.bit.ENABLE = 1;                                                // Enable EIC peripheral
  while (EIC->STATUS.bit.SYNCBUSY);                                        // Wait for synchronization
  
  EVSYS->USER.reg = EVSYS_USER_CHANNEL(1) |                                // Attach the event user (receiver) to channel 0 (n + 1)
                    EVSYS_USER_USER(EVSYS_ID_USER_TC4_EVU);                // Set the event user (receiver) as timer TC4 event 
                    
  EVSYS->CHANNEL.reg = EVSYS_CHANNEL_EDGSEL_NO_EVT_OUTPUT |                // No event edge detection
                       EVSYS_CHANNEL_PATH_ASYNCHRONOUS |                   // Set event path as asynchronous                     
                       EVSYS_CHANNEL_EVGEN(EVSYS_ID_GEN_EIC_EXTINT_3) |    // Set event generator (sender) as external interrupt 3
                       EVSYS_CHANNEL_CHANNEL(0);                           // Attach the generator (sender) to channel 0

  TC4->COUNT32.EVCTRL.reg = TC_EVCTRL_TCEI |         // Enable TC4 event input
                            TC_EVCTRL_EVACT_COUNT;   // Increment the TC4 counter upon receiving an event
                                                                
  TC4->COUNT32.CTRLA.reg = TC_CTRLA_MODE_COUNT32;    // Chain TC4 with TC5 to create a 32-bit timer

  TC4->COUNT32.CTRLA.bit.ENABLE = 1;                 // Enable TC4
  while (TC4->COUNT32.STATUS.bit.SYNCBUSY);          // Wait for synchronization 

  TC4->COUNT32.READREQ.reg = TC_READREQ_RCONT |      // Enable a continuous read request
                             TC_READREQ_ADDR(0x10);   // Offset of the 32-bit COUNT register

//analogWrite(6,127);
// pinMode(10,INPUT_PULLUP);
}

void loop()
{
  delay(2000);
  
  Serial.println("deepsleep");
  delay(1);  
  TC4->COUNT32.COUNT.reg = 0;
  delay(1000);
  //LowPower.deepSleep(2000);
  uint32_t count = TC4->COUNT32.COUNT.reg;// Read the COUNT register
  delay(1);
  Serial.println("wakeup");
  Serial.println(count);
}
