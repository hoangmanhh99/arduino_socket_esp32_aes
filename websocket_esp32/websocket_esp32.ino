#include <Arduino.h>

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>

#include <ArduinoJson.h>

#include <WebSocketsClient.h>
#include <SocketIOclient.h>
#include <ESP.h>
#include <FastLED.h>
#include <HCSR04.h>

#define IN1 22
#define IN2 23
#define ENA 14

#define IN3 27
#define IN4 26
#define ENB 25

#define LED_PIN 13
#define NUM_LEDS 5

byte triggerPin = 16;
byte echoPin = 17;
byte echoCount = 1;
byte* echoPins = new byte[echoCount] { 17 };
CRGB  led[NUM_LEDS];
UltraSonicDistanceSensor distanceSensor(triggerPin, echoPin);

//const char* ssid = "'s Bu Bu";
//const char* password = "24032017";
const char* ssid = "8A4";
const char* password = "123456789";
const char* ip_host = "192.168.1.4";
//const char* ip_host = "arduino-socket-app.herokuapp.com";
const uint16_t port = 3000;
//const uint16_t port = 443;

// Setting PWM properties
const int freq = 1000;
const int pwmChannel1 = 0;
const int pwmChannel2 = 1;
const int resolution = 8;

// direction

//const char F = "F";

WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
SocketIOclient socketIO;

#define USE_SERIAL Serial

void socketIOEvent(socketIOmessageType_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case sIOtype_DISCONNECT:
      USE_SERIAL.printf("[IOc] Disconnected!\n");
      break;
    case sIOtype_CONNECT:
      USE_SERIAL.printf("[IOc] Connected to url: %s\n", payload);

      // join default namespace (no auto join in Socket.IO V3)
      socketIO.send(sIOtype_CONNECT, "/");
      break;
    case sIOtype_EVENT:
      {
        char * sptr = NULL;
        int id = strtol((char *)payload, &sptr, 10);
        USE_SERIAL.printf("[IOc] get event: %s id: %d\n", payload, id);
        if (id) {
          payload = (uint8_t *)sptr;
        }
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, payload, length);
        if (error) {
          USE_SERIAL.print(F("deserializeJson() failed: "));
          USE_SERIAL.println(error.c_str());
          return;
        }

        String eventName = doc[0];
        USE_SERIAL.printf("[IOc] event name: %s\n", eventName.c_str());

        if (eventName == "S") {

          digitalWrite(IN1, 0);
          digitalWrite(IN2, 0);
          ledcWrite(pwmChannel1, 100); // 0-255 ~ 0-12V
          //
          digitalWrite(IN3, 0);
          digitalWrite(IN4, 0);
          ledcWrite(pwmChannel2, 100);
          Serial.println("SSS");
        }
        if (eventName == "F") {

          digitalWrite(IN1, 0);
          digitalWrite(IN2, 1);
          ledcWrite(pwmChannel1, 105); // 0-255 ~ 0-12V
          //
          digitalWrite(IN3, 0);
          digitalWrite(IN4, 1);
          ledcWrite(pwmChannel2, 100);

          Serial.println("FFF");
        }
        if (eventName == "B") {

          digitalWrite(IN1, 1);
          digitalWrite(IN2, 0);
          ledcWrite(pwmChannel1, 103); // 0-255 ~ 0-12V
          //
          digitalWrite(IN3, 1);
          digitalWrite(IN4, 0);
          ledcWrite(pwmChannel2, 100);
          Serial.println("BBB");
        }
        if (eventName == "R") {

          digitalWrite(IN1, 0);
          digitalWrite(IN2, 1);
          ledcWrite(pwmChannel1, 100); // 0-255 ~ 0-12V
          //
          digitalWrite(IN3, 1);
          digitalWrite(IN4, 0);
          ledcWrite(pwmChannel2, 100);
          Serial.println("RRR");

        }

        if (eventName == "L") {

          digitalWrite(IN1, 1);
          digitalWrite(IN2, 0);
          ledcWrite(pwmChannel1, 100); // 0-255 ~ 0-12V
          //
          digitalWrite(IN3, 0);
          digitalWrite(IN4, 1);
          ledcWrite(pwmChannel2, 100);
          Serial.println("LLL");

        }

        if (eventName == "ON") {

          turn_on_led();
          Serial.println("ONNNNN");

        }

        if (eventName == "OFF") {

          turn_off_led();
          Serial.println("OFFFFFF");

        }

        // Message Includes a ID for a ACK (callback)
        if (id) {
          // creat JSON message for Socket.IO (ack)
          DynamicJsonDocument docOut(1024);
          JsonArray array = docOut.to<JsonArray>();

          // add payload (parameters) for the ack (callback function)
          JsonObject param1 = array.createNestedObject();
          param1["now"] = millis();

          // JSON to String (serializion)
          String output;
          output += id;
          serializeJson(docOut, output);

          // Send event
          socketIO.send(sIOtype_ACK, output);
        }
      }
      break;
    case sIOtype_ACK:
      USE_SERIAL.printf("[IOc] get ack: %u\n", length);
      break;
    case sIOtype_ERROR:
      USE_SERIAL.printf("[IOc] get error: %u\n", length);
      break;
    case sIOtype_BINARY_EVENT:
      USE_SERIAL.printf("[IOc] get binary: %u\n", length);
      break;
    case sIOtype_BINARY_ACK:
      USE_SERIAL.printf("[IOc] get binary ack: %u\n", length);
      break;
  }
}

void turn_on_led(void) {
  led[0] = CRGB(255, 0, 0);
  FastLED.show();
}

void turn_off_led(void) {
  led[0] = CRGB(0, 0, 0);
  FastLED.show();
}

void setup() {
  USE_SERIAL.begin(115200);

  USE_SERIAL.setDebugOutput(true);

  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();

  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);

  Serial.println("Hardware info");
  Serial.printf("%d cores Wifi %s%s\n", chip_info.cores, (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
                (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");
  Serial.printf("Silicon revision: %d\n", chip_info.revision);
  Serial.printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
                (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embeded" : "external");

  //get chip id
  String chipId = String((uint32_t)ESP.getEfuseMac(), HEX);
  chipId.toUpperCase();

  Serial.printf("Chip id: %s\n", chipId.c_str());

  FastLED.addLeds<WS2812, LED_PIN, GRB>(led, NUM_LEDS);
  turn_off_led();

  for (uint8_t t = 4; t > 0; t--) {
    USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }

  WiFiMulti.addAP(ssid, password);

  //WiFi.disconnect();
  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(100);
    USE_SERIAL.print(".");
  }
  USE_SERIAL.print("My IP address: ");
  USE_SERIAL.println(WiFi.localIP());

  // use HTTP Basic Authorization this is optional remove if not needed
  //  webSocket.setAuthorization("user", "Password");
  //  socketIO.beginSSL(ip_host, port, "/socket.io/?EIO=4", "HsH");
  socketIO.begin(ip_host, port, "/socket.io/?EIO=4");
  socketIO.onEvent(socketIOEvent);

  // try ever 5000 again if connection has failed
  //  webSocket.setReconnectInterval(5000);

  pinMode(IN1, OUTPUT);  pinMode(IN2, OUTPUT);  pinMode(IN3, OUTPUT);  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);

  ledcSetup(pwmChannel1, freq, resolution);
  ledcAttachPin(ENA, pwmChannel2);

  ledcSetup(pwmChannel2, freq, resolution);
  ledcAttachPin(ENB, pwmChannel1);

}

unsigned long messageTimestamp = 0;

void loop() {
  socketIO.loop();

//  double* distances = HCSR04.measureDistanceCm();
float distances = distanceSensor.measureDistanceCm();

  uint64_t now = millis();

    if(now - messageTimestamp > 2000) {
        messageTimestamp = now;

        // creat JSON message for Socket.IO (event)
        DynamicJsonDocument doc(1024);
        JsonArray array = doc.to<JsonArray>();

        // add evnet name
        // Hint: socket.on('event_name', ....
        array.add("distance");

        // add payload (parameters) for the event
        JsonObject param1 = array.createNestedObject();
        param1["now"] = distances;

        // JSON to String (serializion)
        String output;
        serializeJson(doc, output);

        // Send event
        socketIO.sendEVENT(output);

        // Print JSON for debugging
        USE_SERIAL.println(output);
    }
}
