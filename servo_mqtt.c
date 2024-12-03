#include <WiFi.h>
#include <PubSubClient.h>

const int ledPin = 16;
const int ledChannel = 0;
const int freq = 50;
const int resolution = 16;
int deg, duty;

// Update these with values suitable for your network.
const char* ssid = "ANYS";
const char* password = "87654321";
const char* mqtt_server = "broker.mqtt-dashboard.com";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
int degree = 0;

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String mypayload = "";
  for (int i = 0; i < length; i++) {
    mypayload += (char)payload[i];
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if (mypayload == "1") {
    degree = 180;
    servoWrite(ledChannel, degree);
    delay(3000);  // Wait for 3 seconds
    degree = 0;
    servoWrite(ledChannel, degree);
    Serial.println("Turned to 180 and back to 0 after 3 seconds");
  } else if (mypayload == "2") {
    degree = 0;
    servoWrite(ledChannel, degree);
    Serial.println("2");
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("outTopic", "hello world");
      // ... and resubscribe
      // If you receive something from ESP32, register the subscribe topic
      client.subscribe("bssm");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(ledPin, ledChannel);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  // Maintain the connection to the server
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  unsigned long now = millis();
  if (now - lastMsg > 100) {
    lastMsg = now;
    String mydata = String(degree);
    Serial.print("Publish message: ");
    Serial.println(mydata);
    //client.publish("bssm", mydata.c_str());
  }
}

void servoWrite(int ch, int deg) {
  duty = map(deg, 0, 180, 1638, 8192);
  ledcWrite(ch, duty);
  delay(15);
}
