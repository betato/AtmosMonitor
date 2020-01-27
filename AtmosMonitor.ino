#include "secrets.h"
//const char *ssid = "SSID";
//const char *pass =  "PASS";
//String nameOfCity = "CITY,COUNTRYCODE";
//String apiKey = "APIKEY";

// Display
#include <Wire.h>
#include "SSD1306.h"

// UI
#include "OLEDDisplayUi.h"
#include "images.h"

// WiFi
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>

// Time
#include <NTPClient.h> 
#include <WiFiUdp.h>

// Initialize the OLED display using Wire library
SSD1306 display(0x3c, D3, D5);
OLEDDisplayUi ui (&display);
#define FRAMERATE 30
long framecount = 0;

// Time
const long utcOffset = -18000;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffset);
char daysOfTheWeek[7][12] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

// Weather
float tempNow;
float humidityNow;
String weatherNow;
String weatherLastUpdate = "NEVER";
String city = "Outside";

// Sensors
float temperature;
float humidity;
float temperatureDiscrete;

void overlayClock(OLEDDisplay *display, OLEDDisplayUiState* state) {
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(0, 0, daysOfTheWeek[timeClient.getDay()]);
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->drawString(128, 0, getHoursStr() + (timeClient.getSeconds()%2 ? ":" : " ") + getMinutesStr());
}

void frameTempHumidity(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  if (x == 0 && y == 0) {
    display->drawString(64, 0, "Indoors");
  }
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(x, y + 22, String(temperatureDiscrete, 1) + "C");
  display->drawString(x, y + 38, String(humidity, 0) + "%RH");
  display->drawProgressBar(x + 44, y + 22, 80, 10, (temperatureDiscrete-20)*2 + 50);
  display->drawProgressBar(x + 44, y + 38, 80, 10, humidity);
}

void frameWeatherNow(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  if (x == 0 && y == 0) {
    display->drawString(64, 0, city);
  }
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(x, y + 22, weatherNow);
  display->drawString(x + 64, y + 22, weatherLastUpdate);
  display->drawString(x, y + 38, String(tempNow, 2) + "C");
  display->drawString(x + 64, y + 38, String(humidityNow, 0) + "%RH");
}

FrameCallback frames[] = { frameTempHumidity, frameWeatherNow};
OverlayCallback overlays[] = { overlayClock };

void setup() {
  Serial.begin(115200);
  display.init();
  display.flipScreenVertically();
  WiFi.begin(ssid, pass);

  setupSensors();
  setupTime();
  ui.setTargetFPS(FRAMERATE);
  
  ui.setActiveSymbol(activeSymbol);
  ui.setInactiveSymbol(inactiveSymbol);
  ui.disableAutoTransition();
  ui.setIndicatorPosition(BOTTOM);
  ui.setIndicatorDirection(LEFT_RIGHT);
  ui.setFrameAnimation(SLIDE_LEFT);
  ui.setFrames(frames, 2);
  ui.setOverlays(overlays, 1);
  ui.init(); // Initialising UI will init display too
  
  display.flipScreenVertically();
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  
  updateSensors();
  delay(1000);
  updateWeather();
}

int sensorUpdate = 2 * FRAMERATE;
int clockUpdate = 60 * FRAMERATE;
int weatherUpdate = 240 * FRAMERATE;
int displayTransition = 6 * FRAMERATE;

void loop() {
  int remainingTimeBudget = ui.update();
  if (remainingTimeBudget > 0) {
    // Check if update time has passed
    if (framecount % sensorUpdate == 0) {
      updateSensors();
      Serial.write("s");
    }
    if (framecount % clockUpdate == 0) {
      timeClient.update();
      Serial.write("c");
    }
    if (framecount % weatherUpdate == 0) {
      updateWeather();
      Serial.write("w");
    }
    if (framecount % displayTransition == 0) {
      ui.nextFrame();
      Serial.write("d");
    }
    delay(remainingTimeBudget);
    framecount++;
  }
}
