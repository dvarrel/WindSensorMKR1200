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

#define pinAnemo 4
#define tipTour 2 // 2 aimants
#define R 0.07  //rayon coupelles m
#define DELAY_ANEMO 2000

#define pinGirAdc A6
#define pinGirAlim 5
#define AnalogREF_GIR AR_DEFAULT  //ref voltage 3.3V
#define adcResolutionBits 12
#define nbPos 8
volatile uint16_t count=0;
uint32_t timer;

void isr(){
  count++;
}

void setup()
{
  Serial.begin(115200);                         // Initialise the native serial port
  while(!Serial); // Wait for the console to open
  Serial.println("Setup...");
  pinMode(pinGirAlim,OUTPUT);
  pinMode(pinAnemo,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pinAnemo), isr, FALLING);
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
  


  //anemometre calibration
  static float freq;
  static float v_kmh;
  static uint16_t c;
  uint32_t t = millis() - timer;
  if ( t > DELAY_ANEMO ) {
    c = count;
    count = 0 ;
    timer = millis();
    freq = c / ( tipTour * t / 1000.) ;
    v_kmh = 2. * PI * freq * R * 3.6;
  }
  Serial.print("vitesse=");
  Serial.print(v_kmh);
  Serial.print("km/h \t count=");
  Serial.print(c);
  Serial.print("\t freq=");
  Serial.println(freq);
}
