#ifndef DEF_H_
#define DEF_H_
/*
 * new sensor : TO DO
 * fix calibration 
 * fix bmx280 
*/
#define SOFTVERSION 1
#define SIMUL false
#define CPU_DIVISOR 48 // cpu clock to reduce power 2mA@1Mhz / 14mA@48Mhz
#define CPU_FULL 1
#define Serial Serial1
#define DEBUG false
#define BMx280 true // bmp280 or bme280 installed ?
#define nbPos 8      // 8 capteurs = 8 positions
//see calibation in dev folder
//const uint16_t nGir[nbPos] = {3156, 1952, 686, 991, 1342, 2566, 3781, 3549}; // 922 apremont  
//const uint16_t nGir[nbPos] = {3103, 1890, 658, 951, 1289, 2504, 3750, 3507}; //ID6 925 villes 
//const uint16_t nGir[nbPos] = {3125, 1931, 682, 986, 1330, 2546, 3746, 3518}; //ID1 928 champfro
const uint16_t nGir[nbPos] = {3145, 1962, 690, 994, 1344, 2559, 3766, 3538}; //ID2 931 pres geles

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_BMP280.h>

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
  #define Vdiv (560+390)/390.
#else
  // battery 3V on PB08
  #define pinBatAdc ADC_BATTERY
  #define AnalogREF_BAT AR_INTERNAL1V65 //ref tension pour mesure adc
  #define Vref 1.65
  //voltage divisor R1=680k R2=330k on MKRFOX PB08 Max Battery Voltage = 5.05V
  #define Vdiv (68+33)/33.
#endif

//échantillonnage 3sec d'après WMO
// T = 5 min
#if SIMUL
  #define MS_3S 1000
  #define MS_5MIN 20000
#else
  #define MS_3S 3000
  #define MS_5MIN 5*60*1000
#endif
#define MS_10MIN MS_5MIN*2
#define NB_SIGFOX_DAY 6*24

//anemometre
#define pinAnemo  4     // entrée capteur reed anémomètre
#define TIPTOUR   2     // 2 aimants par tour
#define R         0.07  //rayon coupelles m
#define ANEMO_COEF 3.  //correcteur 1.5
#define BOUNCE_TIME 500 // µs https://www.reed-sensor.com/reed-switches/


//girouette
#define pinGirAdc     A6          // entrée analogique des 8 capteurs reed
#define pinGirAlim    5           // alimentation du réseau de capteurs
#define AnalogREF_GIR AR_DEFAULT  //ref voltage 3.3V
#define angleSlice 360/nbPos
#define DirectionGap 0 // calibrage girouette

//temperature, humidity, pressure
#if BMx280
  #define pinBmx280Vcc 9
  #define pinBmx280Gnd 10
  #define BMx280_I2CADDR 0x76
#endif
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

class Station {
  private:
    uint16_t GirGap = 0xFFFF;
    uint16_t nb_mes = 0;
    bool first_compute = true;
    float s_v;
    double s_sin_g,s_cos_g;
    float v_kmh_min[2] = {999,999};        
    float v_kmh_avg[2];        
    float v_kmh_max[2];        
    uint16_t g_deg_avg[2];
    float batteryVoltage,pressure,temperature,humidity;
    uint8_t encodeWindSpeed (float speedKmh);
    uint8_t encodeWindDirection (uint16_t g_deg);
    uint8_t encodeBatteryVoltage (float v);
    int8_t encodeTemperature(float t);
    uint8_t encodePressure(float p);
    uint8_t encodehumidity(float h);
    bool _debug;
    bool bme280_status = false;
    bool bmp280_status = false;
    Adafruit_BME280 bme280; // I2C
    Adafruit_BMP280 bmp280; // I2C
    void readBatteryVoltage();
    void readBmx280();
    float anemometre(uint16_t count, uint32_t deltaT);
    uint16_t girouette();
    
  public:
    SigfoxWindMessage_t  SigfoxWindMsg;
    void init(bool _debug=false);
    uint16_t get_nbmes();
    bool is_first_compute();
    void set_extra_infos();
    void print_extra_infos();
    void print_sigfox_msg(uint8_t len);
    void compute_measures(uint8_t i);
    void add_measure(uint16_t count, uint32_t deltaT);
    
};

#if DEBUG || CPU_DIVISOR == 1 || SIMUL || BMx280
  #if DEBUG || CPU_DIVISOR == 1 || SIMUL
    #warning "DEBUG MODE"
  #else
    #warning "BMx280 installed"
  #endif
#endif

#endif /* DEF_H_ */
