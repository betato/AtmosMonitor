// Temperature
#include <OneWire.h>
#include <DallasTemperature.h>
// Humidity
#include <DHTesp.h>

OneWire oneWire(D7);
DallasTemperature oneWireSensors(&oneWire);
DHTesp dht;

void setupSensors() {
  dht.setup(D6, DHTesp::DHT11);
}

void updateSensors() {
  temperature = dht.getTemperature();
  humidity = dht.getHumidity();

  oneWireSensors.requestTemperatures();
  temperatureDiscrete = oneWireSensors.getTempCByIndex(0);
}
