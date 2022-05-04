/*
 * esp8266 card to produce wind
 */
void setup() {
  // put your setup code here, to run once:
  pinMode(D5,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  uint16_t t = random(0,20);
  t *=10;
  if(t==0) delay(6500);
  else {
    for(byte i=0;i<10;i++){
      delay(t);
      digitalWrite(D5,1);
      delay(t);
      digitalWrite(D5,0);
    }  
  }
}
