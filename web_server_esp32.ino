#include <LiquidCrystal.h>
#include <Arduino.h>
#ifdef ESP32
  #include <WiFi.h>
  #include <AsyncTCP.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <SPI.h>
#include <Wire.h>
#include "tinyexpr.h"
#include <Adafruit_MCP4725.h>

Adafruit_MCP4725 dac1;
Adafruit_MCP4725 dac2;

#define DAC_RESOLUTION    (9);

LiquidCrystal lcd(19, 23, 18, 17, 16, 15);

AsyncWebServer server(80);

// NETWORK CREDENTIALS
const char* ssid     = "OnePlus Nord CE 2";
const char* password = "e38jrmu2";

const char* PARAM_INPUT_1 = "input1";

int error;

// HTML web page
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <h2>Disang Kriti 2024 Analog<h2>
  <form action="/get">
    INPUT: <input type="text" name="input1"><br>
    <input type="submit" value="Submit">
  </form><br>
  
</body></html>)rawliteral";


void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

double mySum(double a, double b) {
  dac1.setVoltage(a * 4095 / 4.7, false);
  dac2.setVoltage(b * 4095 / 4.7, false);
  int val = analogRead(36);
  double result = ((val * 3.3) / 4095) * 3.10;
  delay(2000);
  return result;
}

double mySub(double a, double b) {
  dac1.setVoltage(a * 4095 / 4.7, false);
  dac2.setVoltage(b * 4095 / 4.7, false);
  int val = analogRead(39);
  double result = ((val * 3.3) / 4095) * 7.0 - 9.8;
  return result;
}

void setup() {
  Serial.begin(115200);
  dac1.begin(0x60);
  dac2.begin(0x61);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
        Serial.print('.');
        lcd.begin(16, 2);
  // Print a message to the LCD.
        lcd.print("Connecting...");
        delay(1000);
      }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Connected");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());

  
  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;

    Serial.println(request->getParam(PARAM_INPUT_1)->value());
    lcd.begin(16, 2);
    lcd.print(request->getParam(PARAM_INPUT_1)->value());
    delay(1000);
    lcd.begin(16, 2);
  // Print a message to the LCD.
    double result = te_interp((request->getParam(PARAM_INPUT_1)->value()).c_str(), &error);
    if (error) {
      lcd.print("Error ");
      lcd.print(error);
    } else {
      lcd.print("Ans ");
      lcd.print(result);
    }
    
    request->send(200, "text/html", "HTTP GET request sent to your ESP on input field (" 
                                     + inputParam + ") with value: " + inputMessage +
                                     "<br><a href=\"/\">Return to Home Page</a>");
  });
  server.onNotFound(notFound);
  server.begin();
}

void loop() {
}