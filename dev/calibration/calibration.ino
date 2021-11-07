/*
 girouette
 misol aliexpress
 adc 12 bits 
*/

#define Serial Serial1

#define pinAnemo 4
#define TIPTOUR 2 // 2 aimants par tour
#define R 0.07  //rayon coupelles m
#define ANEMO_COEF 1.5
#define BOUNCE_TIME 500 // µs https://www.reed-sensor.com/reed-switches/
#define CPU_DIVISOR 1
#define DELAY_ANEMO 5000
#define led LED_BUILTIN

#define pinGirAdc A6
#define pinGirAlim 5
#define AnalogREF_GIR AR_DEFAULT  //ref voltage 3.3V
#define adcResolutionBits 12
#define nbPos 8 //8 reed but 16 differents values ( both detected )
// no high accurate with 16 values, 8 prefered
#define angleSlice 360/nbPos

volatile uint8_t toggle=0;
volatile uint16_t count=0;
volatile uint16_t contactBounceTime;  // Timer to avoid contact bounce in interrupt routine

uint32_t timer;
uint16_t nGir[nbPos]={3092, 1868, 643, 932, 1268, 2485, 3752, 3504};
uint16_t GirGap;
char buffer[128];

void setup()
{
  Serial.begin(9600);  
  while(!Serial); // Wait for the console to open
  Serial.println("Setup...");
  pinMode(pinGirAlim,OUTPUT);
  pinMode(led,OUTPUT);
  pinMode(pinAnemo,INPUT);
  attachInterrupt(digitalPinToInterrupt(pinAnemo), isr_rotation, RISING);
  analogReadResolution(adcResolutionBits);
  analogReference(AnalogREF_GIR);
  timer = millis();
}

void loop()
{
  static uint8_t k = 0;
  
  // girouette calibration
  digitalWrite(pinGirAlim,HIGH);
  delayMicroseconds(10000/CPU_DIVISOR);
  uint16_t m = 0;
  const uint8_t n = 16;
  for(byte i=0;i<n;i++){
    delayMicroseconds(5000/CPU_DIVISOR);
    m = m + analogRead(pinGirAdc);
  }
  digitalWrite(pinGirAlim,LOW);
  m = m / n;
 
  if (Serial.available()) {
    nGir[k] = m;
    k++;
    if (k==nbPos) k = 0;
    while (Serial.available()) Serial.read();    
  }

  GirGap = 0xFFFF;
  for(byte i=0;i<nbPos;i++){
    for(byte j=0;j<nbPos;j++){
      if (nGir[i]!=0 && nGir[j]!=0){
        uint16_t gap = abs(nGir[i] - nGir[j]);
        if ( GirGap > gap && gap > 0 ) GirGap = gap;
      }
    }
  }
  uint16_t x = GirGap / 3;
  uint16_t angle = 0;
  for(byte i=0;i<nbPos;i++){
    if ( m < (nGir[i]+x) && m > (nGir[i]-x)){
      angle = i * angleSlice;
      break;
    }
  }

  snprintf(buffer,sizeof(buffer),"k=%d CAN=%d Gap=%d %d {",k,m,GirGap,angle);
  Serial.print(buffer);
  for(byte i=0;i<nbPos;i++){
      Serial.print(nGir[i]);
      if (i!=nbPos-1) Serial.print(", ");
  }
  Serial.print("}");
  
  uint32_t deltaT = (millis() - timer)* CPU_DIVISOR;
  if ( deltaT > DELAY_ANEMO )
  {
    uint16_t c = count;
    count = 0 ;
    timer = millis();
    float freq = c / ( TIPTOUR * deltaT / 1000.) ;
    float v_kmh = 2. * PI * freq * R * 3.6 * ANEMO_COEF;
    Serial.print(" v=");
    Serial.print(v_kmh);
    Serial.print(" km/h c=");
    Serial.println(c);
  }
  else Serial.println();
    
   
}

void isr_rotation(){
  noInterrupts();
  if (toggle) digitalWrite(led,LOW);
  else digitalWrite(led,HIGH);
  toggle = ~toggle;
  if ((micros() - contactBounceTime) > BOUNCE_TIME/CPU_DIVISOR ) {  // debounce the switch contact.
    count++;
    contactBounceTime = micros();
  }
  interrupts();
}
