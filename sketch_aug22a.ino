#include <WiFiNINA.h>
#include <PubSubClient.h>

// ----------- WiFi Setup -----------
char ssid[] = "bhoomi";       // your WiFi SSID
char pass[] = "bhoominarula";   // your WiFi password

// ----------- MQTT Setup -----------
const char* mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;
const char* topic = "SIT210/wave";

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// ----------- Hardware Pins -----------
const int trigPin = 2;
const int echoPin = 3;
const int ledPin  = 6;

// ----------- Variables -----------
long duration;
int distance;
unsigned long lastPublish = 0;
const int thresholdDistance = 20; // cm

// ----------- WiFi Connect -----------
void setup_wifi() {
  delay(10);
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// ----------- MQTT Callback -----------
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  if (message.indexOf("wave") >= 0) {
    flashLED(3, 500);   // 3 times, 500ms
  } else if (message.indexOf("pat") >= 0) {
    flashLED(5, 200);   // 5 times, fast
  }
}

// ----------- MQTT Reconnect -----------
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ArduinoNano33IoT")) {
      Serial.println("connected");
      client.subscribe(topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// ----------- LED Flash Function -----------
void flashLED(int times, int delayTime) {
  for (int i = 0; i < times; i++) {
    digitalWrite(ledPin, HIGH);
    delay(delayTime);
    digitalWrite(ledPin, LOW);
    delay(delayTime);
  }
}

// ----------- Ultrasonic Distance -----------
int getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2; // cm

  return distance;   // ✅ return so loop() can use it
}


// ----------- Setup -----------
void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ledPin, OUTPUT);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

// ----------- Loop -----------
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int dist = getDistance();
  if (dist > 0 && dist < thresholdDistance) {
    unsigned long now = millis();
    if (now - lastPublish > 3000) { // prevent spamming
      String message = "wave:Bhoomi"; 
      client.publish(topic, message.c_str());
      Serial.print("Published: ");
      Serial.println(message);
      lastPublish = now;
    }
  }
}
