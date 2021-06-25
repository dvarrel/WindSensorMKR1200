#include <SigFox.h>
#define Serial Serial1

void setup() {
  Serial.begin(115200);
  while(!Serial) {};
  if (!SigFox.begin()) {
    Serial.println("Shield error or not present!");
    return;
  }

  String version = SigFox.SigVersion();
  String ID = SigFox.ID();
  String PAC = SigFox.PAC();
  // Display module informations
  Serial.println("MKRFox1200 Sigfox first configuration");
  Serial.println("SigFox FW version " + version);
  Serial.println("ID  = " + ID);
  Serial.println("PAC = " + PAC);
  Serial.println("");
  Serial.print("Module temperature: ");
  Serial.println(SigFox.internalTemperature());
  Serial.println("Register your board on https://backend.sigfox.com/activate with provided ID and PAC");
  delay(100);

  // Send the module to the deepest sleep
  SigFox.end();
}

void loop() {
  // put your main code here, to run repeatedly:

}
