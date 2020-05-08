#include <Arduino.h>
#include <HX711.h>

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <PubSubClient.h>

#include <DHT.h>

const char *mqtt_server = "openhab.home.simcop2387.info";
WiFiManager Wifi;

WiFiClient espClient;
PubSubClient mqtt(espClient);

#define DOUT D1
#define CLK  D2
#define DHTPIN D0

HX711 scale;
DHT dht(DHTPIN, DHT22);

const int averages = 4;

long zero_factor_ch_a = -70000;
long zero_factor_ch_b = -30000;

double calibration_ch_a = -171.75f / (0.35274f / 16.0f);
double calibration_ch_b = -400.0f / (0.35274f / 16.0f); // Guess

double keg_base_ch_a = 15.8f;
double keg_base_ch_b = 15.8f;

double reading_ch_a = 0.0;
double reading_ch_b = 0.0;
double water_ch_a = 0.0;
double water_ch_b = 0.0;
long  raw_ch_a = 0;
long  raw_ch_b = 0;

float temperature = -273.0f;
float humidity = -1.0f;

long last_temp = 0;

void get_readings() {
  // Get the readings
  scale.set_gain(128);
  raw_ch_a = scale.read_average(averages);
  scale.set_gain(32);
  raw_ch_b = scale.read_average(averages);

  // Scale appropriately
  reading_ch_b = ((float) raw_ch_b - zero_factor_ch_b) / calibration_ch_b;
  reading_ch_a = ((float) raw_ch_a - zero_factor_ch_a) / calibration_ch_a;
  
  // Calculate water weight
  water_ch_a = reading_ch_a - keg_base_ch_a;
  water_ch_a = water_ch_a > 0.0 ? water_ch_a : 0.0;

  water_ch_b = reading_ch_b - keg_base_ch_b;
  water_ch_b = water_ch_b > 0.0 ? water_ch_b : 0.0;

  if (millis() - last_temp > 60000) { // Read once per minute
    temperature = dht.readTemperature(true);
    humidity = dht.readHumidity();
    last_temp = millis();
  }
}

void mqtt_reconnect() {
  // Loop until we're reconnected
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "KegWeigh-";
    clientId += String(random(0xffff), HEX);
    Serial.print("\n  Client-id: ");
    Serial.println(clientId);
    Serial.print(" Server: ");
    Serial.println(mqtt_server);
    // Attempt to connect
    if (mqtt.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      mqtt.publish("kegerator/outTopic", "hello world");
      // ... and resubscribe
      mqtt.subscribe("kegerator/inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void setup() {
  Serial.begin(9600);
  // set up the LCD's number of columns and rows:
  Serial.println("HX711 calibration sketch");
  Serial.println("Remove all weight from scale");
  Serial.println("After readings begin, place known weight on scale");
  Serial.println("Press + or a to increase calibration factor");
  Serial.println("Press - or z to decrease calibration factor");

  scale.begin(DOUT, CLK);
  dht.begin();

  Wifi.autoConnect("KegWeigher-MCU", "mandalorian");
  mqtt.setServer(mqtt_server, 1883);
  mqtt.setCallback(mqtt_callback);
}

#define SEND_MQTT(topic, format, var) { \
    length = snprintf(buffer, sizeof(buffer), format, var); \
    mqtt.publish(topic, buffer, length); \
  }

void mqtt_send_readings() {
  char buffer[128];
  long length = 0;

  SEND_MQTT("kegerator/weight_channel_a", "%0.02f", reading_ch_a);
  SEND_MQTT("kegerator/weight_channel_b", "%0.02f", reading_ch_b);
  SEND_MQTT("kegerator/water_channel_a", "%0.02f", water_ch_a);
  SEND_MQTT("kegerator/water_channel_b", "%0.02f", water_ch_b);
  SEND_MQTT("kegerator/raw_channel_a", "%ld", raw_ch_a);
  SEND_MQTT("kegerator/raw_channel_b", "%ld", raw_ch_b);

  
  SEND_MQTT("kegerator/temperature", "%0.02f", temperature);
  SEND_MQTT("kegerator/humidity", "%0.02f", humidity);
}

void loop() {
  if (!mqtt.connected()) {
    mqtt_reconnect();
  }

  mqtt.loop();

  get_readings();

  mqtt_send_readings();

  Serial.print("Readings: ");
  Serial.print(reading_ch_a);
  Serial.print(" lbs, ");
  Serial.print(reading_ch_b);
  Serial.print(" lbs.  water: ");
  Serial.print(water_ch_a);
  Serial.print(" lbs, ");
  Serial.print(water_ch_b);
  Serial.print("lbs  Raw: ");
  Serial.print(raw_ch_a);
  Serial.print(", ");
  Serial.print(raw_ch_b);
  Serial.print(" @ ");
  Serial.print(temperature);\
  Serial.print("F ");
  Serial.print(humidity);
  Serial.print("% ");
  Serial.print(last_temp);
  Serial.println();

}