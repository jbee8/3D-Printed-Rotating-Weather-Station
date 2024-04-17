#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <Stepper.h>
#include <string.h>

const char* ssid = "Enter wifi SSID";
const char* password = "Enter wifi password";

const int stepsPerRevolution = 2048;
const int rolePerMinute = 15;         // Adjustable range of 28BYJ-48 stepper is 0~17 rpm
const char* stepperPosition = "Sun";     // the current position of the stepper motor display in a string format (Sun, Cloud, Rain)
Stepper myStepper(stepsPerRevolution, 12, 14, 13, 15);

// Your Domain name with URL path or IP address with path
String openWeatherMapApiKey = "enter openweathermap api key";
// Example:
//String openWeatherMapApiKey = "bd939aa3d23ff33d3c8f5dd1dd435";

// Replace with your country code and city
String city = "Insert location";
String countryCode = "country code";

// THE DEFAULT TIMER IS SET TO 10 SECONDS FOR TESTING PURPOSES
// For a final application, check the API call limits per hour/minute to avoid getting blocked/banned
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
unsigned long timerDelay = 600000;
// Set timer to 10 seconds (10000)
//unsigned long timerDelay = 10000;

String jsonBuffer;

void updateStepperPosition(JSONVar parsedObject) {
  const char* weatherMain = parsedObject["weather"][0]["main"];
  Serial.print("The weather condition is: ");
  Serial.println(weatherMain);

  if (strcmp(weatherMain, "Clear") == 0 || strcmp(weatherMain, "Hot") == 0) {
    Serial.println("1st if statement test!");
    if (strcmp(stepperPosition, "Sun") == 0) {
      stepperPosition = "Sun";
      Serial.println(stepperPosition);
    } else if (strcmp(stepperPosition, "Cloud") == 0) {
      myStepper.step(-(stepsPerRevolution/3));
      stepperPosition = "Sun";
      Serial.println(stepperPosition);
    } else if (strcmp(stepperPosition, "Rain") == 0) {
      myStepper.step(stepsPerRevolution/3);
      stepperPosition = "Sun";
      Serial.println(stepperPosition);
    }
  } else if (strcmp(weatherMain, "Clouds") == 0 || strcmp(weatherMain, "Mist") == 0 || strcmp(weatherMain, "Haze") == 0 || strcmp(weatherMain, "Smoke") == 0 || strcmp(weatherMain, "Dust") == 0) {
    if (strcmp(stepperPosition, "Sun") == 0) {
      myStepper.step(stepsPerRevolution/3);
      stepperPosition = "Cloud";
      Serial.println(stepperPosition);
    } else if (strcmp(stepperPosition, "Cloud") == 0) {
      stepperPosition = "Cloud";
      Serial.println(stepperPosition);
    } else if (strcmp(stepperPosition, "Rain") == 0) {
      myStepper.step(-(stepsPerRevolution/3));
      stepperPosition = "Cloud";
      Serial.println(stepperPosition);
    }
  } else if (strcmp(weatherMain, "Light Rain") == 0 || strcmp(weatherMain, "Heavy Rain") == 0 || strcmp(weatherMain, "Rain") == 0) {
    if (strcmp(stepperPosition, "Sun") == 0) {
      myStepper.step(-(stepsPerRevolution/3));
      stepperPosition = "Rain";
      Serial.println(stepperPosition);
    } else if (strcmp(stepperPosition, "Cloud") == 0) {
      myStepper.step(stepsPerRevolution/3);
      stepperPosition = "Rain";
      Serial.println(stepperPosition);
    } else if (strcmp(stepperPosition, "Rain") == 0) {
      stepperPosition = "Rain";
      Serial.println(stepperPosition);
    }
  }
}

void getWeatherData() {
  if (WiFi.status() == WL_CONNECTED) {
    String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey;
    //Serial.println(serverPath);

    jsonBuffer = httpGETRequest(serverPath.c_str());
    //Serial.println(jsonBuffer);
    JSONVar myObject = JSON.parse(jsonBuffer);

    // JSON.typeof(jsonVar) can be used to get the type of the var
    if (JSON.typeof(myObject) == "undefined") {
      Serial.println("Parsing input failed!");
      return;
    }

    //Serial.print("JSON object = ");
    //Serial.println(myObject);
    Serial.print("Weather Condition: ");
    Serial.println(myObject["weather"][0]["main"]);
    updateStepperPosition(myObject);
  } else {
    Serial.println("WiFi Disconnected");
  }
}

void setup() {
  myStepper.setSpeed(rolePerMinute);

  Serial.begin(9600);

  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }

  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  getWeatherData();
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    getWeatherData();
    lastTime = millis();
  }
}

String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;

  // Your Domain name with URL path or IP address with path
  http.begin(client, serverName);

  // Send HTTP POST request
  int httpResponseCode = http.GET();

  String payload = "{}";

  if (httpResponseCode > 0) {
    //Serial.print("HTTP Response code: ");
    //Serial.println(httpResponseCode);
    payload = http.getString();
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}
