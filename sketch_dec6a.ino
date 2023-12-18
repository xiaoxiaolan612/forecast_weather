#include "DHTesp.h"
#include <WiFi.h>
#include <PubSubClient.h>

#define mqtt_server "broker.mqttdashboard.com"
#define mqtt_user "lamlam"
#define mqtt_password "Aa123456"
#define LED_PIN     32

unsigned long previousMillis = 0;
const long interval = 5000;
#define humidity_topic "ESP32WI/DHT11/Humidity01"
#define temperature_celsius_topic "ESP32WI/DHT11/Temperature01"
#define MQTT_LED_TOPIC "predict/label"

const char* ssid = "Tang 3";
const char* password = "12345678";

const int DHT_PIN = 14;

WiFiClient wifiClient;
PubSubClient client(wifiClient);
DHTesp dhtSensor;

void setup_wifi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
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


void connect_to_broker() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32WI";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("connected");
      client.subscribe(MQTT_LED_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      delay(2000);
    }
  }
}
void callback(char* topic, byte* payload, unsigned int length) {

  Serial.println("-------new message from broker-----");
  Serial.print("topic: ");
  Serial.println(topic);
  Serial.print("message: ");
  Serial.write(payload, length);
  Serial.println();
  String tempStr;
  for (int i = 0; i < length; i++) {
    tempStr += (char)payload[i];
  }
  if(String(topic) == "predict/label"){
    Serial.println(tempStr);
    if(tempStr == "Yes"){
      
      digitalWrite(LED_PIN,HIGH);
    }
    else digitalWrite(LED_PIN,LOW);
  }
}
void setup() {
  Serial.begin(115200);

  // Kết nối WiFi
  setup_wifi();

  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  connect_to_broker();
  dhtSensor.setup(DHT_PIN, DHTesp::DHT11);
  pinMode(LED_PIN, OUTPUT);
}
void loop() {
  client.loop();
  if (!client.connected()) {
    connect_to_broker();
  }
  
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    TempAndHumidity data = dhtSensor.getTempAndHumidity();
    String ans = String(data.temperature) + "||" + String(data.humidity);
    client.publish(temperature_celsius_topic, ans.c_str());
    // client.publish(humidity_topic, String(data.humidity).c_str());
    previousMillis = currentMillis;
    
    Serial.println("Temp: " + String(data.temperature, 2) + "°C");
    Serial.println("Humidity: " + String(data.humidity, 1) + "%");
    Serial.println("---");
  }
}
