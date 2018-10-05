#include <ESP8266WiFi.h>
#include <PubSubClient.h>


#define HEAT_PIN D2


unsigned long previousMillis = 0;        // will store last time of on command

// constants won't change:
const long interval = 60*60000;           // interval at which to auto disable (milliseconds)

bool state = false;
bool power = false;


// Update these with values suitable for your network.

const char* ssid = "**";
const char* password = "**";
const char* mqtt_server = "192.168.1.15";


WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup() {
  pinMode(HEAT_PIN, OUTPUT);

  digitalWrite(HEAT_PIN, LOW);

  Serial.begin(115200);

  Serial.println("I am heatrelay");
  Serial.print("My timeout is set to ");
  Serial.println(interval);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}



void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    message += (char)payload[i];
  }
  Serial.println();

  String thistopic = String(topic);
  Serial.println(thistopic);

  if (thistopic == "home/heat/power") {
    Serial.println("Incoming Power Instruction");
    Serial.println(message);

    if (message == "on") {
      Serial.println("On!!");
      unsigned long currentMillis = millis();
      previousMillis = currentMillis;
      state = true;
      digitalWrite(HEAT_PIN, HIGH);


    }

    if (message == "off") {
      Serial.println("Off!!");
      digitalWrite(HEAT_PIN, LOW);
    }

  }


}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("heatrelay", "system/controller/heatrelay", 0, false, "disconnected")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("system/controller/heatrelay", "connected");
      // ... and resubscribe
      client.subscribe("home/heat/power");
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

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (state == true) {
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval) {
      // save the last time you blinked the LED
      state = false;
      Serial.println("Timed out!");
      digitalWrite(HEAT_PIN, LOW);

    }
  }

  yield();

}
