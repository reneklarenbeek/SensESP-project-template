#include <WiFi.h>
#include <ESPAsyncWiFiManager.h>
#include "sensesp/signalk/signalk_output.h"
#include "sensesp_app_builder.h"
//sensor
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <OneWire.h>
#include <DallasTemperature.h>


#define oneWireBus 23 //uitlaat voeler op GPIO23
#define BMP_SDA 21 // SDA van BMP280 sensor op GPIO21
#define BMP_SCL 22 // SCL van BMP280 sensor op GPIO22
Adafruit_BMP280 bmp; // Maak een BMP280-object
OneWire OneWire(oneWireBus) ; // maak een oneWire object
DallasTemperature sensors(&OneWire);

//toerenteller
const int pulsCounterPin = 36; // GPIO pin van de pulscounter
volatile int pulseCount =0;
unsigned long lastPulseTime=0;
int rpm=0;

//sensesp
using namespace sensesp;
reactesp::ReactESP app;

unsigned long delaytime = 1000;
SKOutput<float>* luchtdruk_output;
SKOutput<float>* temperatuur_output;
SKOutput<float>* rpm_output;
SKOutput<float>* uitlaat_output;



//----------------------------------------------------------------------------------- void printvalues() -----

void printvalues(){
  float temperatuur = bmp.readTemperature()-2; // Lees de temperatuur in Celsius -2 graden correctie
  float luchtdruk = bmp.readPressure() / 100.0F; // Lees de luchtdruk in hPa (hectopascal)
  sensors.requestTemperatures();
  float uitlaattemp = sensors.getTempCByIndex(0);
 
  temperatuur_output->set_input(temperatuur);
  luchtdruk_output->set_input(luchtdruk);
  uitlaat_output->set_input(uitlaattemp);

  Serial.print("Temperatuur: ");
  Serial.print(temperatuur);
  Serial.println(" °C");
  Serial.print("Luchtdruk: ");
  Serial.print(luchtdruk);
  Serial.println(" hPa");
  Serial.print("Uitlaat temp: ");
  Serial.println(uitlaattemp);
}

//----------------------------------------------------------------------------------- void pulseCounterISR() -----
void pulseCounterISR() {
  pulseCount++;
}

//----------------------------------------------------------------------------------- void PulseCount() -----
void PulseCount(){
  unsigned long currentTime = millis();
  if (currentTime - lastPulseTime >= 1000) {
    rpm = (pulseCount * 60) / ((currentTime - lastPulseTime) / 1000);
    rpm_output->set_input(rpm);
    lastPulseTime = currentTime;
    pulseCount = 0; 
    Serial.print("RPM: ");
    Serial.println(rpm);
   }
}

//----------------------------------------------------------------------------------- void setup() -----

void setup() {
  Serial.begin(115200); // Start de seriële communicatie
  delay(100);  // geef de seriele communicatie even de kans om te starten
  pinMode(pulsCounterPin, INPUT_PULLUP); //pulscounter pinmode
  attachInterrupt(digitalPinToInterrupt(pulsCounterPin), pulseCounterISR, FALLING);
  SensESPAppBuilder builder;
  sensesp_app = (&builder)
            ->set_hostname("sensesp-bmp280")
            ->set_wifi("openplotter","12345678")
            ->get_app();

  sensors.begin() ; //start de communicatie met de temp.voeler
  Wire.begin(BMP_SDA, BMP_SCL); // Start de I2C-communicatie
    if (!bmp.begin(0x76)) { // Het BMP280-adres kan 0x76 of 0x77 zijn, afhankelijk van de sensor
    Serial.println("Kon BMP280 niet vinden. Controleer de verbindingen of het adres.");
    while (1);
  }

    
  luchtdruk_output = new SKOutput<float>(
    "environment.outside.pressure",
    "/sensors/bmp280/pressure",
    new SKMetadata("hPa", "Outside barometric pressure")
    );
  temperatuur_output = new SKOutput<float>(
    "environment.inside.temperature",
    "/sensors/bmp280/temperature",
    new SKMetadata("C", "Cabin temperature")
    );
   rpm_output = new SKOutput<float>(
    "propulsion.engine1.revolutions",
    "/sensors/engine1/rpm",
    new SKMetadata("Hz","Engine RPM")
    );
    uitlaat_output = new SKOutput<float>(
      "propulsion.engine1.exhaustTemperature",
      "/sensors/engine1/exhaustTemperature",
      new SKMetadata("C", "Uitlaat temperatuur")
    );

  //dit moet de laatste regel van void setup() zijn
  sensesp_app->start();

}

//----------------------------------------------------------------------------------- void loop() -----

void loop() {
  static unsigned long last_run = millis();
  if (millis() - last_run >= delaytime){
    printvalues();
    PulseCount();
    last_run = millis();
  }
    app.tick();
}