#include "Arduino.h"
#include "def.h"

void Station::init(bool _debug) {
  globalTick=0;
  tick=0xFF;
  this->_debug = _debug;
  GirSlot = 0xFFFF;
  for(byte i=0;i<nbPos;i++){
    for(byte j=0;j<nbPos;j++){
      if (nGir[i]!=0 && nGir[j]!=0){
        uint16_t x = abs(nGir[i] - nGir[j]);
        if ( GirSlot > x && x > 0 ) GirSlot = x;
      }
    }
  }
  bme280_status = false;
  uint8_t waiting = 0;
  while(!bme280_status && waiting++<100) {
      delay(1);
      bme280_status = bme280.begin(BME280_I2CADDR);      
  }
  if (_debug){
    if (!bme280_status) {
      Serial.println("Could not find BME280 sensor, check wiring, address !");
    }
    else {
      Serial.print(waiting);
      Serial.print(" BME280 ID : 0x");
      Serial.println(bme280.sensorID(),HEX);
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
  g_deg[i]=girouette();

  if (tick ==(NBMES-1) || tick == (NBMES*2-1)){
    synthese();
  }
}

void Station::synthese(){
  uint8_t num = 0;
  if ( tick == NBMES*2-1){
    tick = 0xFF;
    num = 1;
    if (bme280_status) {
      readBme280();
      SigfoxWindMsg.temperature = encodeTemperature(temperature);
      SigfoxWindMsg.pressure = encodePressure(pressure);
      SigfoxWindMsg.humidity = encodehumidity(humidity);
    }
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
      if(g_deg[i] == g_deg[j]) c++;
    }
    if (maxG < c){
      maxG = c;
      g_deg_avg[num] = g_deg[i];
    }
  }

  for(byte i=0;i<2;i++){
    SigfoxWindMsg.speedMin[i]=encodeWindSpeed(v_kmh_min[i]);
    SigfoxWindMsg.speedAvg[i]=encodeWindSpeed(v_kmh_avg[i]);
    SigfoxWindMsg.speedMax[i]=encodeWindSpeed(v_kmh_max[i]);
    SigfoxWindMsg.directionAvg[i]=encodeWindDirection(g_deg_avg[i]);
  }
  
}

float Station::anemometre(uint16_t count, uint32_t deltaT){
    float freq = count / ( TIPTOUR * deltaT / 1000.) ;
    return (2. * PI * freq * R * 3.6);
}

uint16_t Station::girouette(){
  digitalWrite(pinGirAlim,HIGH);
  delayMicroseconds(10000/CPU_DIVISOR);
  uint32_t m=0;
  for(byte i=0;i<10;i++){
    delayMicroseconds(5000/CPU_DIVISOR);
    m = m + analogRead(pinGirAdc);
  }
  digitalWrite(pinGirAlim,LOW);
  m = m / 10;
  uint16_t x = GirSlot / 3;
  for(byte i=0;i<nbPos;i++){
    if ( m < (nGir[i]+x) && m > (nGir[i]-x)){
      return (i * 45) + DirectionGap;
    }
  }
  return 0;
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
    buf += g_deg[i];
    if(i<NBMES-1) buf +=",";
  }
  buf += "]\nv_min= " + String(v_kmh_min[0],2) + ", " + String(v_kmh_min[1],2);
  buf += " \t v_avg= "+ String(v_kmh_avg[0],2) +", "+ String(v_kmh_avg[1],2) ;
  buf += " \t v_max= "+ String(v_kmh_max[0],2) +", "+ String(v_kmh_max[1],2) ;
  buf += " \t g_moy= "+ String(g_deg_avg[0]) +", "+ String(g_deg_avg[1]) ;
  Serial.println(buf);
}

void Station::batteryVoltage() {
  analogReference(AnalogREF_BAT);
  delay(10);
  N = 0;
  for (byte i=0;i<10;i++){
      N = N + analogRead(pinBatAdc);
      delay(5);
  }
  N = N / 10;
  float Vadc = N * Vref / (Nmax);
  u_bat =  Vdiv * Vadc;
  SigfoxWindMsg.batteryVoltage = encodeBatteryVoltage(u_bat);
}

void Station::readBme280(){
  if (bme280_status) {
    temperature = bme280.readTemperature();
    pressure = bme280.readPressure() / 100.;
    humidity = bme280.readHumidity();
  }
}

// encodage vent sur 1 octet (code original du Pioupiou)
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

// encodage direction sur 1 byte ( degres / 2 )
uint8_t Station::encodeWindDirection (uint16_t g_deg) { // degres
  return (uint8_t)(uint16_t)(g_deg / 2);
}

// encodage tension batterie sur 1 octet ( -2. * 100 )
uint8_t Station::encodeBatteryVoltage (float v) {
  return (uint8_t)(float)((v + encodedDeltaVoltage) * 100.);
}

// encodage temperature 1 octet signé (-128 + 127 )
int8_t Station::encodeTemperature(float t) { // radians
  return (int8_t)(t);
}
// encodage pression sur 1 octet (hPa -850)
uint8_t Station::encodePressure(float p) {
  return (uint8_t)(float)(p + encodedGapPressure);
}
// encodage humidité relative sur 1 octet (0-100 %)
uint8_t Station::encodehumidity(float h) {
  return (uint8_t)(h);
}
