#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_BMP280.h>

//#define Serial Serial1

#define pinBmx280Vcc 9
#define pinBmx280Gnd 10
#define BMx280_I2CADDR 0x76

Adafruit_BME280 bme280; // I2C
Adafruit_BMP280 bmp280; // I2C

bool bme280_status = false;
bool bmp280_status = false;

void setup() {
  pinMode(pinBmx280Vcc,OUTPUT);
  pinMode(pinBmx280Gnd,OUTPUT);
  digitalWrite(pinBmx280Vcc,HIGH);
  digitalWrite(pinBmx280Gnd,LOW);
  Serial.begin(115200);
  while(!Serial);    // time to get serial running
  delay(1000);
  Serial.println(F("\nBMx280 test"));
/*
  Wire.begin();
  Wire.beginTransmission(BMx280_I2CADDR);
  byte error = Wire.endTransmission();
  if (error == 0) {
    Serial.print("I2C device found at address 0x");
  }
  else Serial.print("no I2C device found at address 0x");
  Serial.print(BMx280_I2CADDR, HEX);
  Serial.println("  !");
*/  
  uint8_t waiting = 0;
  while(!bme280_status && !bmp280_status && waiting++<100) {
    delay(10);
    bme280_status = bme280.begin(BMx280_I2CADDR);      
    bmp280_status = bmp280.begin(BMx280_I2CADDR);      
  }
  if (!bme280_status && !bmp280_status) {
      Serial.println("Could not find a valid BMx280 sensor, check wiring, address, sensor ID!");
      Serial.print("SensorID was: 0x");
      if (!bme280_status) Serial.print(bme280.sensorID(),HEX);
      if (!bmp280_status) Serial.println(bmp280.sensorID(),HEX);
      Serial.print("ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
      Serial.print("ID of 0x56-0x58 represents a BMP 280,\n");
      Serial.print("ID of 0x60 represents a BME 280.\n");
      Serial.print("ID of 0x61 represents a BME 680.\n");
      while (1) delay(10);
  }
  Serial.print("SensorID was: 0x");
  if (bme280_status){
    Serial.println(bme280.sensorID(),HEX);
  }
  else{
    Serial.println(bmp280.sensorID(),HEX);
  }
  
  Serial.println("-- Default Test --");
}


void loop() { 
    printValues();
    delay(2000);
}

void printValues() {
    Serial.print("Temperature = ");
    if (bme280_status) Serial.print(bme280.readTemperature());
    else Serial.print(bmp280.readTemperature());
    Serial.println(" °C");

    Serial.print("Pressure = ");

    if (bme280_status) Serial.print(bme280.readPressure() / 100.0F);
    else Serial.print(bmp280.readPressure() / 100.0F);
    Serial.println(" hPa");

    if (bme280_status) {
      Serial.print("Humidity = ");
      Serial.print(bme280.readHumidity());
      Serial.println(" %");
    }

    Serial.println();
}
