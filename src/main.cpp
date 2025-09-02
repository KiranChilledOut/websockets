#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>

WiFiClient client;
WebSocketsClient wsClient;

#define WIFI_SSID "kautomateFriends"
#define WIFI_PASSWORD "Sunilpro@11"
#define WS_HOST ""
#define WS_PORT 443
#define WS_URL ""
#define MSG_SIZE 256

void sendOkMessage()
{
  wsClient.sendTXT("{\"action\":\"msg\",\"type\":\"status\",\"body\":\"ok\"}");
}

uint8_t toMode(const char *mode)
{
  if (strcmp(mode, "INPUT") == 0)
    return INPUT;
  else if (strcmp(mode, "OUTPUT") == 0)
    return OUTPUT;
  else if (strcmp(mode, "INPUT_PULLUP") == 0)
    return INPUT_PULLUP;
  else
    return INPUT;
}

void sendErrorMessage(const char *error)
{
  char msg[MSG_SIZE];
  sprintf(msg, "{\"action\":\"msg\",\"type\":\"error\",\"body\":\"%s\"}", error);
  wsClient.sendTXT(msg);
}

void handleWebSocketMessage(uint8_t *payload)
{
  JsonDocument doc;

  DeserializationError error = deserializeJson(doc, payload);

  // Test if parsing succeeds
  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    sendErrorMessage(error.c_str());
    return;
  }

  if (!doc["type"].is<const char *>())
  {
    sendErrorMessage("invalid message: missing type");
    return;
  }

  if (strcmp(doc["type"], "cmd") == 0)
  {
    if (!doc["body"].is<JsonObject>())
    {
      sendErrorMessage("invalid cmd: missing body");
      return;
    }
    if (strcmp(doc["body"]["type"], "pinMode") == 0)
    {
      pinMode(doc["body"]["pin"], toMode(doc["body"]["mode"]));
      sendOkMessage();
      return;
    }
    if (strcmp(doc["body"]["type"], "digitalWrite") == 0)
    {
      digitalWrite(doc["body"]["pin"], toMode(doc["body"]["mode"]));
      sendOkMessage();
      return;
    }
    if (strcmp(doc["body"]["type"], "digitalRead") == 0)
    {
      int pinState = digitalRead(doc["body"]["pin"]);
      char msg[MSG_SIZE];
      sprintf(msg, "{\"action\":\"msg\",\"type\":\"pinState\",\"body\":{\"pin\":%d,\"state\":%d}}", doc["body"]["pin"].as<int>(), pinState);
      wsClient.sendTXT(msg);
      return;
    }
    sendErrorMessage("unknown cmd type");
    return;
  }
  else if (strcmp(doc["type"], "command") == 0)
  {
    if (!doc["body"].is<const char *>())
    {
      sendErrorMessage("invalid command: missing body");
      return;
    }
    Serial.printf("Command: %s\n", doc["body"].as<const char *>());
  }
  else
  {
    sendErrorMessage("unknown message type");
  }
}

void onWSEvent(WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:
    Serial.println("WebSocket Disconnected");
    break;
  case WStype_CONNECTED:
    Serial.println("WebSocket Connected");
    // Send a message to the server
    wsClient.sendTXT("Hello from ESP32");
    break;
  case WStype_TEXT:
    Serial.printf("WebSocket Message: %s\n", payload);
    handleWebSocketMessage(payload);
    break;
  case WStype_BIN:
    Serial.printf("WebSocket Binary Message of length: %u\n", length);
    break;
  case WStype_PING:
    // pong will be send automatically
    Serial.println("WebSocket Ping");
    break;
  case WStype_PONG:
    // answer to a ping we send
    Serial.println("WebSocket Pong");
    break;
  case WStype_ERROR:
    Serial.println("WebSocket Error");
    break;
  default:
    Serial.println("WebSocket Unknown Event");
  }
}

void setup()
{

  Serial.begin(921600); // Changed from 921600 to 9600 for PlatformIO compatibility
  pinMode(LED_BUILTIN, OUTPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting to WiFi...");

  wsClient.beginSSL(WS_HOST, WS_PORT, WS_URL, "", "wss");
  wsClient.onEvent(onWSEvent);
}

bool isConnected = false;

void loop()
{
  if (WiFi.status() == WL_CONNECTED && !isConnected)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("Connected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    isConnected = true;
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println(".");
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(1000);
    isConnected = false;
  }

  wsClient.loop();
}
