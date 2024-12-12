#include <NTPClient.h>

/***************************************************************************
  This is a library for the BME680 gas, humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BME680 Breakout
  ----> http://www.adafruit.com/products/3660

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

#include <Wire.h>
#include <SPI.h>
#include <stdio.h>
#include <stdlib.h>

// MQTT -- following guide https://community.hivemq.com/t/hivemq-using-esp32-and-nodered/1291
#include <WiFi.h>
#include <PubSubClient.h>
// #include <WiFiClientSecure.h> //TODO

// #define BME_SCK 21
// #define BME_MISO 23
// #define BME_MOSI 22
// #define BME_CS 5

// For motion detector
const int fan_relay = 4;
unsigned long now = millis();
unsigned long lastTrigger = 0;
boolean startTimer = false;
int motion = 0;
float current_temp = 0;



// //---- WiFi settings //TODO
const char* ssid = "ry";
const char* password = "dbxx2a58";

// //---- MQTT Broker settings //TODO
// const char* mqtt_server = "2bfae14d28a14fc48d06fb6bd71f180d.s1.eu.hivemq.cloud"; // replace with your broker url
// const char* mqtt_username = "swarangi";
// const char* mqtt_password = "Sanhita1";
// const int mqtt_port = 8883;

const char* mqtt_server = "192.168.118.55"; // replace with your broker url
const char* mqtt_username = "admin-user";
const char* mqtt_password = "admin-password";
const int mqtt_port = 30083;

WiFiClient espClient; //TODO
PubSubClient client(espClient);
unsigned long lastMsg = 0;

#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

 const char* temp_topic = "sensors/temperature";
 const char* humid_topic = "sensors/humidity";
 const char* motion_topic = "sensors/motion";


void setup() {
  Serial.begin(115200);

  pinMode(fan_relay, OUTPUT);
  digitalWrite(fan_relay, LOW);

  setup_wifi();
  // espClient.setInsecure(); //TODO
  
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  while (!Serial) {delay(1);}
}

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

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Gateway: ");
  Serial.println(WiFi.gatewayIP());
  Serial.println(WiFi.subnetMask());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageStr;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageStr += (char)message[i];
  }
  Serial.println();


  if (String(topic) == String(motion_topic)) {
    char *endptr;
    motion = strtof(messageStr.c_str(), &endptr);
  }

  if (String(topic) == String(temp_topic)) {
    char *endptr;
    current_temp = strtof(messageStr.c_str(), &endptr);
  }

  if (String(topic) == String(humid_topic)) {
    char *endptr;
    float current_humid = strtof(messageStr.c_str(), &endptr);
    if (((current_humid > 70 and current_temp > 25) or current_humid > 80 or current_temp > 27) and motion == 1){
      digitalWrite(fan_relay, HIGH);
    } else {
      digitalWrite(fan_relay, LOW);
    };
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // if (client.connect("ESP8266Client")) {
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");
      // Subscribe
      client.subscribe(temp_topic);
      client.subscribe(humid_topic);
      client.subscribe(motion_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()){ reconnect();}
  client.loop();
  delay(2000);
}
