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
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

// MQTT -- following guide https://community.hivemq.com/t/hivemq-using-esp32-and-nodered/1291
#include <WiFi.h>
#include <PubSubClient.h>
// #include <WiFiClientSecure.h>

// #define BME_SCK 21
// #define BME_MISO 23
// #define BME_MOSI 22
// #define BME_CS 5

#define SEALEVELPRESSURE_HPA (1013.25)
#define timeSeconds 5

// For motion detector
const int motion1_pin = 27;
// const int motion2_pin = 5;
const int led = 26;

unsigned long now = millis();
unsigned long lastTrigger = 0;
boolean startTimer = false;
boolean motion = false;

Adafruit_BME680 bme; // I2C
// Adafruit_BME680 bme(BME_CS); // hardware SPI
// Adafruit_BME680 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK);

// //---- WiFi settings
const char* ssid = "ry";
const char* password = "dbxx2a58";

// //---- MQTT Broker settings
// const char* mqtt_server = "2bfae14d28a14fc48d06fb6bd71f180d.s1.eu.hivemq.cloud"; // replace with your broker url
// const char* mqtt_username = "swarangi";
// const char* mqtt_password = "Sanhita1";
// const int mqtt_port = 8883;

const char* mqtt_server = "192.168.118.55"; // replace with your broker url
const char* mqtt_username = "admin-user";
const char* mqtt_password = "admin-password";
const int mqtt_port = 30083;

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;

#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

 const char* temp_topic = "sensors/temperature";
 const char* humid_topic = "sensors/humidity";
 const char* motion_topic = "sensors/motion";


void IRAM_ATTR detectsMovement() {
  digitalWrite(led, HIGH);
  startTimer = true;
  lastTrigger = millis();
}

void setup() {
  Serial.begin(115200);
  // Serial.begin(9600);

  // PIR Motion Sensor mode INPUT_PULLUP
  pinMode(motion1_pin, INPUT_PULLUP);
  // pinMode(motion2_pin, INPUT_PULLUP);
  // Set motionSensor pin as interrupt, assign interrupt function and set RISING mode
  attachInterrupt(digitalPinToInterrupt(motion1_pin), detectsMovement, RISING);
  // attachInterrupt(digitalPinToInterrupt(motion2_pin), detectsMovement, RISING);

  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);

  while (!Serial);
  Serial.println(F("BME680 async test"));

  if (!bme.begin()) {
    Serial.println(F("Could not find a valid BME680 sensor, check wiring!"));
    while (1);
  }

  setup_wifi();
  // espClient.setInsecure(); // Use this only for testing, it allows connecting without a root certificate
  
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);

  randomSeed(micros());

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
  // Serial.print("Message arrived on topic: ");
  // Serial.print(topic);
  // Serial.print(". Message: ");
  // String messageTemp;
  
  // for (int i = 0; i < length; i++) {
  //   Serial.print((char)message[i]);
  //   messageTemp += (char)message[i];
  // }
  // Serial.println();

  // // Feel free to add more if statements to control more GPIOs with MQTT

  // // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // // Changes the output state according to the message
  // if (String(topic) == "esp32/output") {
  //   Serial.print("Changing output to ");
  //   if(messageTemp == "on"){
  //     Serial.println("on");
  //     digitalWrite(ledPin, HIGH);
  //   }
  //   else if(messageTemp == "off"){
  //     Serial.println("off");
  //     digitalWrite(ledPin, LOW);
  //   }
  // }
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

  // Motion detection
  now = millis();
  if((digitalRead(led) == HIGH) && (motion == false)) {
    Serial.println("MOTION DETECTED!!!");
    char* motionString;
    motionString = "1";
    client.publish(motion_topic, motionString, true);
    motion = true;
  }
  // Turn off the LED after the number of seconds defined in the timeSeconds variable
  if(startTimer && (now - lastTrigger > (timeSeconds*1000))) {
    Serial.println("Motion stopped...");
    char* nomotionString;
    nomotionString = "0";
    client.publish(motion_topic, nomotionString, true);
    digitalWrite(led, LOW);
    startTimer = false;
    motion = false;
  }

  // Tell BME680 to begin measurement.
  unsigned long endTime = bme.beginReading();
  if (endTime == 0) {
    Serial.println(F("Failed to begin reading :("));
    return;
  }
  Serial.print(F("Reading started at "));
  Serial.print(millis());
  Serial.print(F(" and will finish at "));
  Serial.println(endTime);

  Serial.println(F("You can do other work during BME680 measurement."));
  delay(50); // This represents parallel work.
  // There's no need to delay() until millis() >= endTime: bme.endReading()
  // takes care of that. It's okay for parallel work to take longer than
  // BME680's measurement time.

  // Obtain measurement results from BME680. Note that this operation isn't
  // instantaneous even if milli() >= endTime due to I2C/SPI latency.
  if (!bme.endReading()) {
    Serial.println(F("Failed to complete reading :("));
    return;
  }
  Serial.print(F("Reading completed at "));
  Serial.println(millis());

  Serial.print(F("Temperature = "));
  Serial.print(bme.temperature);
  Serial.println(F(" *C"));

  Serial.print(F("Humidity = "));
  Serial.print(bme.humidity);
  Serial.println(F(" %"));

  char tempString[8];
  dtostrf(bme.temperature, 1, 2, tempString);
  client.publish(temp_topic, tempString);

  char humString[8];
  dtostrf(bme.humidity, 1, 2, humString);
  client.publish(humid_topic, humString);

  Serial.println();
  delay(5000);
}
