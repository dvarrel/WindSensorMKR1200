#include "Arduino.h"
#include "def.h"

void Station::init(bool _debug) {
  globalTick=0;
  tick=0xFF;
  this->_debug = _debug;
  gap = 0xFFFF;
  for(byte i=0;i<nbPos;i++){
    for(byte j=0;j<nbPos;j++){
      if (nGir[i]!=0 && nGir[j]!=0){
        uint16_t x = abs(nGir[i] - nGir[j]);
        if ( gap > x && x > 0 ) gap = x;
      }
    }
  }
}

void Station::add_measure(uint16_t count, uint32_t deltaT){
  globalTick++;
  tick++;
  
  uint8_t i = tick % NBMES;
  if(_debug){
    snprintf(buffer,sizeof(buffer),"tick=%d ",tick);
    Serial.print(buffer);
  }
  v_kmh[i]=anemometre(count,deltaT);
  g_rad[i]=girouette();

  if (tick ==(NBMES-1) || tick == (NBMES*2-1)){
    synthese();
  }
}

void Station::synthese(){
  uint8_t num = 0;
  if ( tick == NBMES*2-1){
    tick = 0xFF;
    num = 1;
  }
  v_kmh_min[num] = v_kmh[0];
  v_kmh_max[num] = v_kmh[0];
  float sum = v_kmh[0];
  
  for(byte i=1;i<NBMES;i++){
    if (v_kmh_min[num]>v_kmh[i])
      v_kmh_min[num] = v_kmh[i];
    if (v_kmh_max[num] < v_kmh[i]) 
      v_kmh_max[num] = v_kmh[i];
    sum = sum + v_kmh[i];
  }
  v_kmh_avg[num] = sum/NBMES;

  uint8_t maxG = 0;
  for(byte i=0;i<NBMES;i++){
    uint8_t c=0;
    for(byte j=0;j<NBMES;j++){
      if(g_rad[i] == g_rad[j]) c++;
    }
    if (maxG < c){
      maxG = c;
      g_rad_avg[num] = g_rad[i];
    }
  }

  for(byte i=0;i<2;i++){
    SigfoxWindMessage.speedMin[i]=encodeWindSpeed(v_kmh_min[i]);
    SigfoxWindMessage.speedAvg[i]=encodeWindSpeed(v_kmh_avg[i]);
    SigfoxWindMessage.speedMax[i]=encodeWindSpeed(v_kmh_max[i]);
    SigfoxWindMessage.directionAvg[i]=encodeWindDirection(g_rad_avg[i]);
  }
  
}

float Station::anemometre(uint16_t count, uint32_t deltaT){
    float freq = count / ( tipTour * deltaT / 1000.) ;
    return 2. * PI * freq * R * 3.6;
}

float Station::girouette(){
  digitalWrite(pinGirAlim,HIGH);
  delayMicroseconds(10000/CPU_DIVISOR);
  uint32_t m=0;
  for(byte i=0;i<10;i++){
    delayMicroseconds(5000/CPU_DIVISOR);
    m = m + analogRead(pinGirAdc);
  }
  digitalWrite(pinGirAlim,LOW);
  m = m / 10;
  uint16_t x = gap / 3;
  for(byte i=0;i<nbPos;i++){
    if ( m < (nGir[i]+x) && m > (nGir[i]-x)){
      return (i * 2. * PI / 8);
    }
  }
}

float Station::get_V(){
  return v_kmh[tick%NBMES];
}

float Station::get_G(){
  return g_rad[tick%NBMES];
}

void Station::print(){
  buf="#### mesures n°"; 
  buf += String(globalTick);
  buf += " ####\nv=[";
  for(byte i=0;i<NBMES;i++){
    buf += v_kmh[i];
    if(i<NBMES-1) buf +=",";
  }
  buf +="] \t g=[";
  for(byte i=0;i<NBMES;i++){
    buf += g_rad[i];
    if(i<NBMES-1) buf +=",";
  }
  buf += "]\nv_min= " + String(v_kmh_min[0],2) + ", " + String(v_kmh_min[1],2);
  buf += " \t v_avg= "+ String(v_kmh_avg[0],2) +", "+ String(v_kmh_avg[1],2) ;
  buf += " \t v_max= "+ String(v_kmh_max[0],2) +", "+ String(v_kmh_max[1],2) ;
  buf += " \t g_moy= "+ String(g_rad_avg[0],2) +", "+ String(g_rad_avg[1],2) ;
  Serial.println(buf);
}

void Station::batteryVoltage() {
  analogReference(AnalogREF_BAT);
  delay(10);
  N = 0;
  for (byte i=0;i<10;i++){
      N = N + analogRead(ADC_BATTERY);
      delay(5);
  }
  N = N / 10;
  float Vadc = N * Vref / (Nmax);
  u_bat =  Vdiv * Vadc;
}

// pour l'encodage de la tension batterie sur 1 octet
uint8_t Station::encodeBatteryVoltage (float Vbat) {
  return (uint8_t)(float)((Vbat-2) * 100);
}

// pour l'encodage du vent sur 1 byte
// (code original du Pioupiou)
uint8_t Station::encodeWindSpeed (float speedKmh) {
  uint8_t encodedSpeed;
  if (speedKmh < 10.) {
    // 0 to 9.75 kmh : 0.25 km/h resolution
    encodedSpeed = (uint8_t)(float)(speedKmh * 4. + 0.5);
  } else if (speedKmh < 80.) {
    // 10 to 79.5 kmh  : 0.5 km/h resolution
    encodedSpeed = (uint8_t)(float)(speedKmh * 2. + 0.5) + 20;
  } else if (speedKmh < 120.) {
    // 80 to 119 kmh  : 1 km/h resolution
    encodedSpeed = (uint8_t)(float)(speedKmh + 0.5) + 100;
  } else if (speedKmh < 190.) {
    // 120 to 188 kmh  : 2 km/h resolution
    encodedSpeed = (uint8_t)(float)(speedKmh / 2. + 0.5) + 160;
  } else {
    // 190 or + : out of range
    encodedSpeed = 0xFF;
  }
  return encodedSpeed;
}

// pour l'encodage de la direction sur 1 byte
uint8_t Station::encodeWindDirection (float g_rad) { // radians
  float direction = g_rad / M_PI * 180.;   // radians to degrees
  if (direction < 0.) direction += 360.;  // -180-180 to 0-360+

  // encode with 2° precision
  // add 0.5 for rounding when converting from (float) to (int)
  return (uint8_t)(float)(direction / 2. + 0.5);
}
