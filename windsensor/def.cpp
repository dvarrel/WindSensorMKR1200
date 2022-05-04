#include "Arduino.h"
#include "def.h"
#include <math.h>

void Station::init(bool _debug) {
  this->_debug = _debug;
  for(byte i=0;i<nbPos;i++){
    for(byte j=0;j<nbPos;j++){
      if (nGir[i]!=0 && nGir[j]!=0){
        uint16_t gap = abs(nGir[i] - nGir[j]);
        if ( GirGap > gap && gap > 0 ) GirGap = gap;
      }
    }
  }

  #if BMx280
  uint8_t waiting = 0;
  while(!bme280_status && !bmp280_status && waiting++<100) {
      delay(10);
      bme280_status = bme280.begin(BMx280_I2CADDR);      
      bmp280_status = bmp280.begin(BMx280_I2CADDR);      
  }
  if (_debug) {
    if (!bme280_status && !bmp280_status){
      Serial.println("Could not find BMx280 sensor...");
    }
    else {
      Serial.print(waiting);
      Serial.print(" Sensor found : ");
      if (bme280_status) Serial.println("BME280");
      else Serial.println("BMP280");
    }
  }
  if (bme280_status || bmp280_status){
    this->readBmx280();    
  }
  #endif
  if (_debug){
    Serial.println("station init end");
    Serial.flush();
  }
}

void Station::add_measure(uint16_t count, uint32_t deltaT){
  nb_mes++;
  uint8_t i = first_compute;
  uint16_t g_deg = girouette();
  float v = anemometre(count,deltaT);
  
  if(_debug){
    char b[128];
    snprintf(b,sizeof(b),"#n°%d# v=%.1f g=%d",nb_mes,v,g_deg);
    Serial.println(b);
    Serial.flush();
  }
  
  if (v_kmh_min[i] > v) v_kmh_min[i]=v;
  if (v_kmh_max[i] < v) v_kmh_max[i]=v;
  s_v += v;
  
  s_sin_g += v * sin(g_deg*PI/180.);
  s_cos_g += v * cos(g_deg*PI/180.);
}

void Station::compute_measures(uint8_t i){
  v_kmh_avg[i] = s_v / nb_mes;
  double g = atan2(s_sin_g , s_cos_g)*180./PI;
  if (g<0) g += 360;
  g_deg_avg[i] = int(g);
  
  SigfoxWindMsg.speedMin[i] = encodeWindSpeed(v_kmh_min[i]);
  SigfoxWindMsg.speedAvg[i] = encodeWindSpeed(v_kmh_avg[i]);
  SigfoxWindMsg.speedMax[i] = encodeWindSpeed(v_kmh_max[i]);
  SigfoxWindMsg.directionAvg[i] = encodeWindDirection(g_deg_avg[i]);
  if (_debug) {
    String buf = "v_min=" + String(v_kmh_min[i],1);
    buf += "\t v_avg="+ String(v_kmh_avg[i],1);
    buf += "\t v_max="+ String(v_kmh_max[i],1);
    buf += "\t g_moy="+ String(g_deg_avg[i])+" ";
    Serial.println(buf);
    Serial.flush();
  }
  
  v_kmh_min[i]=999;
  v_kmh_max[i]=0;
  s_v =0; s_sin_g=0; s_cos_g=0;
  nb_mes=0;
  first_compute = !first_compute;
}

uint16_t Station::get_nbmes(){
  return this->nb_mes;
}

bool Station::is_first_compute(){
  return this->first_compute;
}

float Station::anemometre(uint16_t count, uint32_t deltaT){
    float freq = count / ( TIPTOUR * deltaT / 1000.) ;
    float v = 2. * PI * freq * R * 3.6 * ANEMO_COEF;
    return v;
}

uint16_t Station::girouette(){
  digitalWrite(pinGirAlim,HIGH);
  delayMicroseconds(10000/CPU_DIVISOR);
  uint16_t m=0;
  const uint8_t n = 16;
  for(byte i=0;i<n;i++){
    delayMicroseconds(5000/CPU_DIVISOR);
    m = m + analogRead(pinGirAdc);
  }
  digitalWrite(pinGirAlim,LOW);
  m = m / n;
  uint16_t x = GirGap / 3;
  for(byte i=0;i<nbPos;i++){
    if ( m < (nGir[i]+x) && m > (nGir[i]-x)){
      return (i * angleSlice) + DirectionGap;
    }
  }
  return 0;
}

void Station::print_extra_infos(){
  String s = "Ubat=" +String((SigfoxWindMsg.batteryVoltage/100.)-encodedDeltaVoltage,2)+"V";
  s += "\tT="+ String(SigfoxWindMsg.temperature)+"°C";
  s += "\tP="+ String(SigfoxWindMsg.pressure - encodedGapPressure)+"hPa";
  s += "\tH="+ String(SigfoxWindMsg.humidity>>1)+"%";
  s += "\tsigfox_last_error=" + String(SigfoxWindMsg.lastMessageStatus); 
  Serial.println(s);
  Serial.flush();
}

void Station::print_sigfox_msg(uint8_t len){
  String s= String(len)+"bytes = ";
  char buffer[64];
  snprintf(buffer, sizeof(buffer), "%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X",
    SigfoxWindMsg.speedMin[0],SigfoxWindMsg.speedMin[1],
    SigfoxWindMsg.speedAvg[0],SigfoxWindMsg.speedAvg[1],
    SigfoxWindMsg.speedMax[0],SigfoxWindMsg.speedMax[1],
    SigfoxWindMsg.directionAvg[0],SigfoxWindMsg.directionAvg[1]);
    s += String(buffer);
  if (len==12) {
    snprintf(buffer, sizeof(buffer), ",%02X,%02X,%02X,%02X",
    SigfoxWindMsg.batteryVoltage,
    SigfoxWindMsg.pressure,
    SigfoxWindMsg.temperature,
    SigfoxWindMsg.humidity);
    s += String(buffer);
  }
  Serial.println(s);
  Serial.flush();
}

void Station::readBatteryVoltage() {
  analogReference(AnalogREF_BAT);
  delayMicroseconds(10000/CPU_DIVISOR);
  uint16_t N = 0;
  const uint8_t n = 8;
  for (byte i=0; i<n; i++){
      N += analogRead(pinBatAdc);
      delayMicroseconds(5000/CPU_DIVISOR);
  }
  N /= n;
  float Vadc = N * Vref / (Nmax);
  batteryVoltage =  Vdiv * Vadc;
  SigfoxWindMsg.batteryVoltage = encodeBatteryVoltage(batteryVoltage);
}

void Station::readBmx280(){
  if (bme280_status) {
    temperature = bme280.readTemperature();
    pressure = bme280.readPressure() / 100.;
    humidity = bme280.readHumidity();
  }
  else if(bmp280_status) {
    temperature = bmp280.readTemperature();
    pressure = bmp280.readPressure() / 100.;
  }
}

void Station::set_extra_infos(){
    readBatteryVoltage();
    readBmx280();
    SigfoxWindMsg.temperature = encodeTemperature(temperature);
    SigfoxWindMsg.pressure = encodePressure(pressure);
    SigfoxWindMsg.humidity = encodehumidity(humidity);
    // add last error to pressure byte -> error=0 pressure even else odd
    if (SigfoxWindMsg.lastMessageStatus==0)
      SigfoxWindMsg.pressure &= 0xFE;
    else
      SigfoxWindMsg.pressure |= 0x01;
    //add softversion to humidity
    SigfoxWindMsg.humidity = (SigfoxWindMsg.humidity<<1) + SOFTVERSION;
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
  return (int8_t)(round(t));
}
// encodage pression sur 1 octet (hPa -850)
uint8_t Station::encodePressure(float p) {
  return (uint8_t)(uint16_t)(round(p) + encodedGapPressure);
}
// encodage humidité relative sur 1 octet (0-100 %)
uint8_t Station::encodehumidity(float h) {
  return (uint8_t)(round(h));
}
