#include <ArduinoJson.h>

const char weatherServer[] = "api.openweathermap.org";
int status = WL_IDLE_STATUS;
WiFiClient client;

bool updateWeather() {
  // Close any existing connections
  client.stop();

  if (client.connect(weatherServer, 80)) {
    client.println("GET /data/2.5/forecast?q=" + nameOfCity + "&APPID=" + apiKey + "&mode=json&units=metric&cnt=2 HTTP/1.1");
    client.println("Host: api.openweathermap.org");
    client.println("User-Agent: ESP8266");
    client.println("Connection: close");
    client.println();
    
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println("Weather client timeout");
        client.stop();
        return false;
      }
    }
    
    char c = 0;
    int jsonend = 0;
    boolean startJson = false;
    String text;
    while (client.available()) {
      c = client.read();
      // Count brackets to find end or message
      if (c == '{') {
        startJson = true;
        jsonend++;
      } else if (c == '}') {
        jsonend--;
      }
      if (startJson == true) {
        text += c;
      }
      if (jsonend == 0 && startJson == true) {
        // Equal number of brackets 
        bool parseSuccess = parseJson(text.c_str());
        return parseSuccess;
      }
    }
    Serial.println("Could not read full json: " + text);
    return false;
  }
  else {
    Serial.println("Weather connection failed");
    return false;
  }
}

bool parseJson(const char * jsonString) {
  const size_t bufferSize = 2*JSON_ARRAY_SIZE(1) + JSON_ARRAY_SIZE(2) + 4*JSON_OBJECT_SIZE(1) + 3*JSON_OBJECT_SIZE(2) + 3*JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + 2*JSON_OBJECT_SIZE(7) + 2*JSON_OBJECT_SIZE(8) + 720;
  DynamicJsonBuffer jsonBuffer(bufferSize);

  JsonObject& root = jsonBuffer.parseObject(jsonString);
  if (!root.success()) {
    Serial.println("Json parsing failed");
    return false;
  }

  JsonArray& list = root["list"];
  JsonObject& nowT = list[0];

  city = root["city"]["name"].as<String>();;
  tempNow = nowT["main"]["temp"];
  humidityNow = nowT["main"]["humidity"];
  weatherNow = nowT["weather"][0]["description"].as<String>();
  weatherLastUpdate = getHoursStr() + ":" + getMinutesStr();
  
  return true;
}
