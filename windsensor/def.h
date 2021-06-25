#ifndef DEF_H_
#define DEF_H_

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define Serial Serial1
#define DEBUG false
#define SigFox12bytes false

#define Battery18650
#define encodedDeltaVoltage -2
#define adcResolutionBits 12  
#define Nmax pow(2, adcResolutionBits)-1

#ifdef Battery18650

//battery 3.7V on A4
#define pinBatAdc     A4    
#define AnalogREF_BAT AR_DEFAULT  //ref voltage 3.3V
#define Vref 3.3
//voltage divisor R1=560k R2=390k Max Battery Voltage = 7.9V
#define Vdiv (555+394)/394.

#else
// battery on PB08
#define pinBatAdc ADC_BATTERY
#define AnalogREF_BAT AR_INTERNAL1V65 //ref tension pour mesure adc
#define Vref 1.65
//voltage divisor R1=680k R2=330k on MKRFOX PB08 Max Battery Voltage = 5.05V
#define Vdiv (68+33)/33.

#endif


// cpu clock to reduce power 2mA@1Mhz / 14mA@48Mhz
#define CPU_DIVISOR 1
#define FULL 1
//10 mesures en 5 min -> delai 30000
#define TICK_DELAY 30000/CPU_DIVISOR
#define TICK_DAY 144  // 144 messages per day

//anemometre
#define pinAnemo  4     // entrée capteur reed anémomètre
#define TIPTOUR   2     // 2 aimants par tour
#define R         0.07  //rayon coupelles m
#define BOUNCE_TIME 500 // µs https://www.reed-sensor.com/reed-switches/


//girouette
#define pinGirAdc     A6          // entrée analogique des 8 capteurs reed
#define pinGirAlim    5           // alimentation du réseau de capteurs
#define AnalogREF_GIR AR_DEFAULT  //ref voltage 3.3V
#define nbPos 8                   // 8 capteurs = 8 positions
const uint16_t nGir[8]={1135, 2501, 3771, 3533, 3129, 1830, 367, 728};
#define DirectionGap 0 // calibrage girouette

//temperature, humidity, pressure
#define pinBme280Vcc 9
#define pinBme280Gnd 10
#define BME280_I2CADDR 0x76
#define encodedGapPressure -850


// pour transmettre 2 périodes / message :
// message de 12 bytes
typedef struct __attribute__ ((packed)) sigfox_wind_message {
        uint8_t speedMin[2];
        uint8_t speedAvg[2];
        uint8_t speedMax[2];
        uint8_t directionAvg[2];
        uint8_t batteryVoltage;
        uint8_t pressure;
        int8_t temperature;
        uint8_t humidity;
        uint8_t lastMessageStatus;
} SigfoxWindMessage_t;
// /!\ avec ce format, il faut transmettre
// impérativement deux périodes de 5 minutes
// xxx[0] -> de T-10 à T-5 minutes
// xxx[1] -> de T-5 minutes à T
// T étant l'heure de transmission

#define NBMES 10
class Station {
  private:
    uint16_t GirSlot;
    uint32_t globalTick;
    char buffer[128];
    String buf;
    uint8_t encodeWindSpeed (float speedKmh);
    uint8_t encodeWindDirection (uint16_t g_deg);
    uint8_t encodeBatteryVoltage (float v);
    int8_t encodeTemperature(float t);
    uint8_t encodePressure(float p);
    uint8_t encodehumidity(float h);
    bool _debug;
    bool bme280_status;
    Adafruit_BME280 bme280; // I2C
    
  public:
    uint8_t tick;
    float v_kmh[NBMES];
    uint16_t g_deg[NBMES];
    float v_kmh_min[2];        
    float v_kmh_avg[2];        
    float v_kmh_max[2];        
    uint16_t g_deg_avg[2];
    float u_bat;
    uint16_t N;
    float pressure,temperature,humidity;
    SigfoxWindMessage_t  SigfoxWindMsg;

    void init(bool _debug=false);
    void print();
    void synthese();
    void batteryVoltage();
    void readBme280();
    void add_measure(uint16_t count, uint32_t deltaT);
    float anemometre(uint16_t count, uint32_t deltaT);
    uint16_t girouette();
};

#endif /* DEF_H_ */
