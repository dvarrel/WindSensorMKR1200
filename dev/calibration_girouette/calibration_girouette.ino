/*
 girouette type lextronic
 mi sol aliexpress
 adc 10 bits
 uint8_t nGir[8]={283, 181, 91, 456, 781, 883, 941, 624};
 minGap = 59
 adc 12 bits 
 uint16_t nGir[8]={1135, 728, 367, 1830, 3129, 3533, 3771, 2501};
 minGap=235
*/

//#define Serial Serial1

#define pinGirAdc A6
#define pinGirAlim 5
#define AnalogREF_GIR AR_DEFAULT  //ref voltage 3.3V
#define adcResolutionBits 12
#define nbPos 8
uint32_t timer;

void setup()
{
  Serial.begin(115200);  
  while(!Serial); // Wait for the console to open
  Serial.println("Setup...");
  pinMode(pinGirAlim,OUTPUT);
  analogReadResolution(adcResolutionBits);
  analogReference(AnalogREF_GIR); 
  timer = millis();
}

void loop()
{
  // girouette calibration
  static uint16_t nGir[nbPos];
  digitalWrite(pinGirAlim,HIGH);
  delay(10);
  uint32_t m=0;
  for(byte i=0;i<20;i++){
    delay(5);
    m = m + analogRead(pinGirAdc);
  }
  digitalWrite(pinGirAlim,LOW);
  m = m / 20;
  Serial.println(m);
  if (Serial.available()) {
    for(byte i=0;i<nbPos;i++){
      if (nGir[i]==0 ){
        nGir[i]= m;
        break;
      }  
    }
    while (Serial.available()) Serial.read();    
  }
  
  Serial.print("[");
  for(byte i=0;i<nbPos;i++){
      Serial.print(nGir[i]);
      if (i!=nbPos-1) Serial.print(", ");
  }
  Serial.print("] min_gap=");

  static uint16_t minGap = 0xFFFF;
  for(byte i=0;i<nbPos;i++){
    for(byte j=0;j<nbPos;j++){
      if (nGir[i]!=0 && nGir[j]!=0){
        uint16_t gap = abs(nGir[i] - nGir[j]);
        if ( minGap > gap && gap > 0 ) minGap = gap;
      }
    }
  }
  Serial.println(minGap);
  
}
