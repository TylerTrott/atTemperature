#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <WiFiClientSecure.h>
#include <SPIFFS.h>
#include "at_client.h"
#include "dht_nonblocking.h"
#include "constants.h"

#define DHT_SENSOR_PIN 4
#define DHT_SENSOR_TYPE DHT11


DHT dht(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

// atSigns and keys
const auto *at_sign = new AtSign("@esp");

//TODO: change this to my own atSign
const auto *java = new AtSign("@java");
const auto keys = keys_reader::read_keys(*at_sign); 

void setup() {
  Serial.begin(9600);
  // reads the keys on the ESP32
    const auto keys = keys_reader::read_keys(*at_sign); 
    
    // creates the AtClient object (allows us to run operations)
    auto *at_client = new AtClient(*at_sign, keys);  
    
    // pkam authenticate into our atServer
    at_client->pkam_authenticate(SSID, PASSWORD); 

  // Initialize DHT sensor
  dht.begin();
}

void loop() {
  
  // Wait a few seconds between measurements.
  delay(2000);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));

  //convert floats to strings
  std::__cxx11::string temperature = std::to_string(t);
  std::__cxx11::string humidity = std::to_string(h);
  std::__cxx11::string heat_index = std::to_string(hic);

  // Send data to atServer
  auto *at_client = new AtClient(*at_sign, keys);

  // send temperature
  const auto *temperature_key = new AtKey("temperature", at_sign, java);

  at_client->put_ak(*temperature_key, temperature);

  // send humidity
  const auto *humidity_key = new AtKey("humidity", at_sign, java);
  
  at_client->put_ak(*humidity_key, humidity);

  // send heat index
  const auto *heat_index_key = new AtKey("heat_index", at_sign, java);

  at_client->put_ak(*heat_index_key, heat_index);
}