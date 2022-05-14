#include <WiFi.h>
#include <PubSubClient.h>  // Download and install this library first from: https://www.arduinolibraries.info/libraries/pub-sub-client
#include <WiFiClient.h>

#define SSID_NAME "Rao-WiFi"                 // Your Wifi Network name
#define SSID_PASSWORD "pintu2512"         // Your Wifi network password
#define MQTT_BROKER "smartnest.cz"            // Broker host
#define MQTT_PORT 1883                        // Broker port
#define MQTT_USERNAME "lakshyarao22"              // Username from Smartnest
#define MQTT_PASSWORD "lakshya22"              // Password from Smartnest (or API key)
#define MQTT_CLIENT "617c443f71143e77f621aedc"               // Device Id from smartnest
#define FIRMWARE_VERSION "Tutorial-Doorbell"  // Custom name for this program

WiFiClient espClient;
PubSubClient client(espClient);
int bellPin = 15;
int LEDB = 13;
bool bellTriggered = false;
int bellReportSend = 0;

int brightness = 0;
int fadeAmount = 5;
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 10;


void startWifi();
void startMqtt();
void sendBellReport();
void checkBell();
void checkMqtt();
int splitTopic(char* topic, char* tokens[], int tokensNumber);
void callback(char* topic, byte* payload, unsigned int length);
void sendToBroker(char* topic, char* message);


void callback(char* topic, byte* payload, unsigned int length) {  // This function runs when there is a new message in the subscribed topic
  Serial.print("Topic:");
  Serial.println(topic);

  // Splits the topic and gets the message
  int tokensNumber = 10;
  char* tokens[tokensNumber];
  char message[length + 1];
  splitTopic(topic, tokens, tokensNumber);
  sprintf(message, "%c", (char)payload[0]);

  for (int i = 1; i < length; i++) {
    sprintf(message, "%s%c", message, (char)payload[i]);
  }

  Serial.print("Message:");
  Serial.println(message);
}

void startWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID_NAME, SSID_PASSWORD);
  Serial.println("Connecting ...");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 10) {
    attempts++;
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println('\n');
    Serial.print("Connected to ");
    Serial.println(WiFi.SSID());
    Serial.print("IP address:\t");
    Serial.println(WiFi.localIP());

  } else {
    Serial.println('\n');
    Serial.println('I could not connect to the wifi network after 10 attempts \n');
  }

  delay(500);
}

void startMqtt() {
  client.setServer(MQTT_BROKER, MQTT_PORT);
  client.setCallback(callback);

  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");

    if (client.connect(MQTT_CLIENT, MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.println("connected");
    } else {
      if (client.state() == 5) {
        Serial.println("Connection not allowed by broker, possible reasons:");
        Serial.println("- Device is already online. Wait some seconds until it appears offline for the broker");
        Serial.println("- Wrong Username or password. Check credentials");
        Serial.println("- Client Id does not belong to this username, verify ClientId");
        esp_restart();

      } else {
        Serial.println("Not possible to connect to Broker Error code:");
        Serial.print(client.state());
        esp_restart();
      }

      delay(0x7530);
    }
  }

  char subscibeTopic[100];
  sprintf(subscibeTopic, "%s/#", MQTT_CLIENT);
  client.subscribe(subscibeTopic);  //Subscribes to all messages send to the device

  sendToBroker("report/online", "true");  // Reports that the device is online
  delay(100);
  sendToBroker("report/firmware", FIRMWARE_VERSION);  // Reports the firmware version
  delay(100);
  sendToBroker("report/ip", (char*)WiFi.localIP().toString().c_str());  // Reports the ip
  delay(100);
  sendToBroker("report/network", (char*)WiFi.SSID().c_str());  // Reports the network name
  delay(100);

  char signal[5];
  sprintf(signal, "%d", WiFi.RSSI());
  sendToBroker("report/signal", signal);  // Reports the signal strength
  delay(100);
}

int splitTopic(char* topic, char* tokens[], int tokensNumber) {
  const char s[2] = "/";
  int pos = 0;
  tokens[0] = strtok(topic, s);

  while (pos < tokensNumber - 1 && tokens[pos] != NULL) {
    pos++;
    tokens[pos] = strtok(NULL, s);
  }

  return pos;
}

void checkMqtt() {
  if (!client.connected()) {
    startMqtt();
  }
}

void checkBell() {
  int buttonState = digitalRead(bellPin);
  //digitalWrite(lightPin, digitalRead(bellPin));
  if (buttonState == HIGH && !bellTriggered) {
    return;

  } else if (buttonState == HIGH && bellTriggered) {
    bellTriggered = false;

  } else if (buttonState == LOW && !bellTriggered) {
    bellTriggered = true;
    sendBellReport();
    breathingBlue();

  } else if (buttonState == LOW && bellTriggered) {
    return;
  }
}

void sendBellReport() {  //Avoids sending repeated reports. only once every 5 seconds.
  if (millis() - bellReportSend > 5000) {
    sendToBroker("report/detectionState", "true");
    bellReportSend = millis();
  }
}

void sendToBroker(char* topic, char* message) {
  if (client.connected()) {
    char topicArr[100];
    sprintf(topicArr, "%s/%s", MQTT_CLIENT, topic);
    client.publish(topicArr, message);
  }
}

void breathingBlue(){
  
  pinMode(LEDB,OUTPUT);
  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(LEDB, ledChannel);
  for(int i = 0; i < 820; i++)
  {
    ledcWrite(ledChannel, brightness);
    brightness = brightness + fadeAmount;
    if (brightness <= 0 || brightness >= 1023) {
      fadeAmount = -fadeAmount;
    }
    delay(5);
  }
}

void setup() {
  pinMode(bellPin, INPUT_PULLDOWN);
  pinMode(LEDB, OUTPUT);
  Serial.begin(115200);
  startWifi();
  startMqtt();
}

void loop() {
  client.loop();
  checkBell();
  checkMqtt();
}
