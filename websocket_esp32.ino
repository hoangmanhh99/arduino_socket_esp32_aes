#include <WiFi.h>
//#include <WebSocketsServer.h>
#include <WebSocketsClient.h>

const char* ssid = "8A4";
const char* password = "123456789";

// Globals
WebSocketsClient webSocket;

// Called when receiving any WebSocket message

void hexdump(const void *mem, uint32_t len, uint8_t cols = 16) {
  const uint8_t* src = (const uint8_t*) mem;
  Serial.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
  for (uint32_t i = 0; i < len; i++) {
    if (i % cols == 0) {
      Serial.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
    }
    Serial.printf("%02X ", *src);
    src++;
  }
  Serial.printf("\n");
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[WSc] Disconnected!\n");
      break;
    case WStype_CONNECTED:
//      {
//        IPAddress ip = webSocket.remoteIP(num);
//        Serial.printf("{%u} Connection from ", num);
//        Serial.println(ip.toString());
//      }
      Serial.printf("[WSc] Connected to url: %s\n", payload);
      webSocket.sendTXT("Connected");
      break;

    case WStype_TEXT:
      Serial.printf("[WSc] get text: %s\n", payload);
//      hexdump(payload, length);
//      webSocket.sendTXT(num, payload);
      break;

    case WStype_BIN:
      Serial.printf("[WSc] get binary length: %u\n", length);
      hexdump(payload, length);
      break;
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT: 
    case WStype_FRAGMENT_FIN:
    default:
      break;
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  Serial.println("Connecting");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Connected!");
  Serial.print("My IP address: ");
  Serial.println(WiFi.localIP());

  webSocket.begin("arduino-socket-app.herokuapp.com", 29216, "/");
  webSocket.onEvent(webSocketEvent);

  webSocket.setReconnectInterval(5000);
}

void loop() {
  webSocket.loop();

}
