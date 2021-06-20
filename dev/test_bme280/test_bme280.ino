#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define Serial Serial1

#define pinBme280Vcc 9
#define pinBme280Gnd 10
#define BME280_I2CADDR 0x76

Adafruit_BME280 bme; // I2C


unsigned long delayTime;

void setup() {
  pinMode(pinBme280Vcc,OUTPUT);
  pinMode(pinBme280Gnd,OUTPUT);
  digitalWrite(pinBme280Vcc,HIGH);
  digitalWrite(pinBme280Gnd,LOW);
    Serial.begin(115200);
    while(!Serial);    // time to get serial running
    Serial.println(F("BME280 test"));

    bool bme280_status = false;
    uint8_t waiting = 0;
    while(!bme280_status && waiting++<90) {
      delay(1);
      bme280_status = bme.begin(BME280_I2CADDR);      
    }

    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
        Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
        Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
        Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
        Serial.print("        ID of 0x60 represents a BME 280.\n");
        Serial.print("        ID of 0x61 represents a BME 680.\n");
        while (1) delay(10);
    }
    else Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
    
    Serial.println("-- Default Test --");
    delayTime = 1000;

    Serial.println();
}


void loop() { 
    printValues();
    delay(delayTime);
}


void printValues() {
    Serial.print("Temperature = ");
    Serial.print(bme.readTemperature());
    Serial.println(" °C");

    Serial.print("Pressure = ");

    Serial.print(bme.readPressure() / 100.0F);
    Serial.println(" hPa");

    Serial.print("Humidity = ");
    Serial.print(bme.readHumidity());
    Serial.println(" %");

    Serial.println();
}
