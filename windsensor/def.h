#ifndef DEF_H_
#define DEF_H_

#define Serial Serial1
#define DEBUG true


// battery on PB08
#define adcResolutionBits 12  
#define Nmax pow(2, adcResolutionBits)-1
#define AnalogREF_BAT AR_INTERNAL1V65 //ref tension pour mesure adc
#define Vref 1.65
//voltage divisor R1=680k R2=330k on MKRFOX PB08
#define Vdiv (68+33)/33.

// cpu clock to reduce power 2mA@1Mhz / 14mA@48Mhz
#define CPU_DIVISOR 48
#define FULL 1
//10 mesures en 5 min -> delai 30000
#define TICK_DELAY 1000/CPU_DIVISOR

//anemometre
#define pinAnemo  4     // entrée capteur reed anémomètre
#define tipTour   2     // 2 aimants par tour
#define R         0.07  //rayon coupelles m

//girouette
#define pinGirAdc     A6          // entrée analogique des 8 capteurs reed
#define pinGirAlim    5           // alimentation du réseau de capteurs
#define AnalogREF_GIR AR_DEFAULT  //ref voltage 3.3V
#define nbPos 8                   // 8 capteurs = 8 positions
const uint16_t nGir[8]={1135, 728, 367, 1830, 3129, 3533, 3771, 2501};

// pour transmettre 2 périodes / message :
// message de 8 bytes
typedef struct __attribute__ ((packed)) sigfox_wind_message {
        uint8_t speedMin[2];
        uint8_t speedAvg[2];
        uint8_t speedMax[2];
        uint8_t directionAvg[2];
        uint8_t batteryVoltage;
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
    uint16_t gap;
    uint32_t globalTick;
    char buffer[128];
    String buf;
    uint8_t encodeWindSpeed (float speedKmh);
    uint8_t encodeWindDirection (float g_rad);
    uint8_t encodeBatteryVoltage (float Vbat);
    bool _debug;
    
  public:
    uint8_t tick;
    float v_kmh[NBMES];
    float g_rad[NBMES];
    float v_kmh_min[2];        
    float v_kmh_avg[2];        
    float v_kmh_max[2];        
    float g_rad_avg[2];
    float u_bat;
    uint16_t N;
    SigfoxWindMessage_t  SigfoxWindMessage;

    void init(bool _debug=false);
    void print();
    void synthese();
    void batteryVoltage();
    void add_measure(uint16_t count, uint32_t deltaT);
    float anemometre(uint16_t count, uint32_t deltaT);
    float girouette();
    float get_V();
    float get_G();
};

#endif /* DEF_H_ */
