
#include <WiFi.h>
#include <ESPAsyncWiFiManager.h>
#include "sensesp/signalk/signalk_output.h"
#include "sensesp_app_builder.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

#define BMP_SDA 21 // Verander dit naar de pin waarop SDA is aangesloten
#define BMP_SCL 22 // Verander dit naar de pin waarop SCL is aangesloten

Adafruit_BMP280 bmp; // Maak een BMP280-object

using namespace sensesp;
reactesp::ReactESP app;

unsigned long delaytime = 1000;
SKOutput<float>* luchtdruk_output;
SKOutput<float>* temperatuur_output;

void printvalues(){
  float temperatuur = bmp.readTemperature(); // Lees de temperatuur in Celsius
  float luchtdruk = bmp.readPressure() / 100.0F; // Lees de luchtdruk in hPa (hectopascal)
  temperatuur_output->set_input(temperatuur);
  luchtdruk_output->set_input(luchtdruk);

  Serial.print("Temperatuur: ");
  Serial.print(temperatuur);
  Serial.println(" °C");
  Serial.print("Luchtdruk: ");
  Serial.print(luchtdruk);
  Serial.println(" hPa");
}

void setup() {
  Serial.begin(115200); // Start de seriële communicatie
  delay(100);  // geef de seriele communicatie even de kans om te starten

  SensESPAppBuilder builder;
  sensesp_app = (&builder)
            ->set_hostname("sensesp-bmp280")
            ->set_wifi("openplotter","12345678")
            ->get_app();

  Wire.begin(BMP_SDA, BMP_SCL); // Start de I2C-communicatie
  if (!bmp.begin(0x76)) { // Het BMP280-adres kan 0x76 of 0x77 zijn, afhankelijk van de sensor
    Serial.println("Kon BMP280 niet vinden. Controleer de verbindingen of het adres.");
    while (1);
  }
  
  luchtdruk_output = new SKOutput<float>(
    "environment.cabin.pressure",
    "/sensors/bmp280/pressure",
    new SKMetadata("Pa", "Cabin barometric pressure")
    );
  temperatuur_output = new SKOutput<float>(
    "environment.cabin.temperature",
    "/sensors/bmp280/temperature",
    new SKMetadata("C", "Cabin temperature")
    );


  sensesp_app->start();

}
void loop() {
  static unsigned long last_run = millis();
  if (millis() - last_run >= delaytime){
    printvalues();
    last_run = millis();
  }
  
  app.tick();
}