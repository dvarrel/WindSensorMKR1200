/*
*/

//#define Serial Serial1

#define pinAnemo 4
#define tipTour 2 // 2 aimants
#define R 0.07  //rayon coupelles m
#define BOUNCE_TIME 500 // µs https://www.reed-sensor.com/reed-switches/
#define DELAY_ANEMO 5000

#define CPU_DIVISOR 1
#define led LED_BUILTIN

volatile uint8_t toggle=0;
volatile uint16_t count=0;
volatile uint16_t contactBounceTime;  // Timer to avoid contact bounce in interrupt routine

uint32_t timer;
uint16_t tour;

void isr(){
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

void setup()
{
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Setup...");
  pinMode(led,OUTPUT);
  pinMode(pinAnemo,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pinAnemo), isr, RISING);
  timer = millis();
  tour=1;
}

void loop()
{
  //anemometre calibration
  static float freq;
  static float v_kmh;
  static uint16_t c;
  static uint16_t lastcount;
  /*if(lastcount != count) {
    Serial.print(count);
    lastcount=count;
  }*/
  uint32_t t = millis() - timer;
  if ( t > DELAY_ANEMO ) {
    c = count;
    count = 0 ;
    timer = millis();
    freq = c / ( tipTour * t / 1000.) ;
    v_kmh = 2. * PI * freq * R * 3.6;
    Serial.print(tour);
    Serial.print("\t vitesse=");
    Serial.print(v_kmh);
    Serial.print("km/h \t count=");
    Serial.print(c);
    Serial.print("\t freq=");
    Serial.println(freq);
    tour++;
  }
}
