#include <Arduino.h>

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>

#include <ArduinoJson.h>

#include <WebSocketsClient.h>
#include <SocketIOclient.h>

#define IN1 2
#define IN2 15
#define ENA 4

#define IN3 18
#define IN4 19
#define ENB 5

//const char* ssid = "'s Bu Bu";
//const char* password = "24032017";
const char* ssid = "Meetingroom T3";
const char* password = "0938992396";
//const char* ip_host = "192.168.1.12";
const char* ip_host = "arduino-socket-app.herokuapp.com";
//const uint16_t port = 3000;
const uint16_t port = 443;

// Setting PWM properties
const int freq = 30000;
const int pwmChannel1 = 0;
const int pwmChannel2 = 5;
const int resolution = 8;

// direction

//const char F = "F";

WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
SocketIOclient socketIO;

#define USE_SERIAL Serial

void hexdump(const void *mem, uint32_t len, uint8_t cols = 16) {
  const uint8_t* src = (const uint8_t*) mem;
  USE_SERIAL.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
  for (uint32_t i = 0; i < len; i++) {
    if (i % cols == 0) {
      USE_SERIAL.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
    }
    USE_SERIAL.printf("%02X ", *src);
    src++;
  }
  USE_SERIAL.printf("\n");
}

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

          digitalWrite(IN1, 1);
          digitalWrite(IN2, 0);
          ledcWrite(pwmChannel1, 100); // 0-255 ~ 0-12V
          //
          digitalWrite(IN3, 1);
          digitalWrite(IN4, 0);
          ledcWrite(pwmChannel2, 100);

          Serial.println("FFF");
        }
        if (eventName == "B") {

          digitalWrite(IN1, 0);
          digitalWrite(IN2, 1);
          ledcWrite(pwmChannel1, 100); // 0-255 ~ 0-12V
          //
          digitalWrite(IN3, 0);
          digitalWrite(IN4, 1);
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

void setup() {
  // USE_SERIAL.begin(921600);
  USE_SERIAL.begin(115200);

  //Serial.setDebugOutput(true);
  USE_SERIAL.setDebugOutput(true);

  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();

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

  // server address, port and URL
  //  webSocket.begin(ip_host, port, "/");

  // event handler
  //  webSocket.onEvent(webSocketEvent);

  // use HTTP Basic Authorization this is optional remove if not needed
  //  webSocket.setAuthorization("user", "Password");
    socketIO.beginSSL(ip_host, port, "/socket.io/");
//  socketIO.begin(ip_host, port, "/socket.io/?EIO=4");
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

void loop() {
  socketIO.loop();
}
