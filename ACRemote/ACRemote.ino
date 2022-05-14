#include <WiFi.h>
//#include <ESP8266WiFi.h>
#include <PubSubClient.h>  // Download and install this library first from: https://www.arduinolibraries.info/libraries/pub-sub-client
#include <WiFiClient.h>
#include <IRremote.h>

#define SSID_NAME "Rao-2.4"              // Your Wifi Network name
#define SSID_PASSWORD "pintu2512"      // Your Wifi network password
#define MQTT_BROKER "smartnest.cz"         // Broker host
#define MQTT_PORT 1883                     // Broker port
#define MQTT_USERNAME "lakshyarao22"           // Username from Smartnest
#define MQTT_PASSWORD "lakshya22"           // Password from Smartnest (or API key)
#define MQTT_CLIENT "624ff7e6dd3b74129b7b4db0"            // Device Id from smartnest
#define FIRMWARE_VERSION "AC"  // Custom name for this program

WiFiClient espClient;
PubSubClient client(espClient);

const byte IR_LED = 13;

IRsend irsend(IR_LED);

unsigned int rawOn[162] = {1050,550,1050,550,1050,2500,1050,2500,1100,550,1050,550,1000,2550,1050,2500,1100,550,1050,550,1050,2500,1050,550,1050,2500,1100,500,1100,500,1100,500,1050,2500,1100,550,1050,500,1100,500,1100,500,1100,2450,1100,2500,1100,2450,1100,550,1100,500,1050,550,1050,2500,1100,2450,1150,500,1100,450,1100,500,1100,500,1100,500,1100,2450,1150,2450,1100,2450,1150,500,1100,2450,1100,2450,1100,550,1050,500,1100,2500,1050,2500,1100,2500,1050,550,1050,2500,1100,2500,1050,550,1050,550,1050,2500,1100,2500,1050,2500,1050,550,1050,2550,1050,2500,1050,550,1050,550,1050,550,1050,2500,1050,600,1000,550,1050,550,1050,2550,1050,550,1000,600,1000,550,1050,550,1050,550,1050,550,1050,550,1050,550,1000,600,1000,2550,1050,550,1050,550,1050,550,1000,600,1000,2550,1050,2550,1000};
unsigned int rawOff[162] = {1050,550,1050,550,1050,2550,1000,2550,1050,550,1050,550,1050,2550,1000,2550,1050,550,1050,550,1050,2500,1050,550,1050,2550,1050,500,1100,550,1000,600,1000,600,1000,550,1050,550,1050,550,1050,550,1050,2550,1000,2550,1050,2550,1050,550,1000,600,1000,550,1050,2550,1050,2550,1000,550,1050,550,1050,550,1050,550,1050,550,1050,2550,1000,2550,1050,2550,1050,500,1050,2550,1050,2550,1050,550,1050,550,1000,2550,1050,2550,1050,2500,1050,550,1050,2550,1000,2550,1050,550,1050,550,1000,2600,1050,2500,1050,2550,1050,550,1050,2500,1050,2550,1050,550,1050,550,1050,550,1000,2550,1050,550,1050,550,1050,550,1050,2500,1050,550,1050,550,1050,550,1050,550,1050,550,1050,550,1000,600,1000,550,1050,2550,1050,2550,1000,600,1000,550,1050,550,1050,550,1050,2550,1000,2550,1050};

void startWifi();
void startMqtt();
void checkMqtt();
int splitTopic(char* topic, char* tokens[], int tokensNumber);
void callback(char* topic, byte* payload, unsigned int length);
void sendToBroker(char* topic, char* message);

void turnOff();
void turnOn();

void callback(char* topic, byte* payload, unsigned int length) {  //A new message has been received
	Serial.print("Topic:");
	Serial.println(topic);
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

	if (strcmp(tokens[1], "directive") == 0 && strcmp(tokens[2], "powerState") == 0) {
		if (strcmp(message, "ON") == 0) {
			turnOn();
		} else if (strcmp(message, "OFF") == 0) {
			turnOff();
		}
	}
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

void sendToBroker(char* topic, char* message) {
	if (client.connected()) {
		char topicArr[100];
		sprintf(topicArr, "%s/%s", MQTT_CLIENT, topic);
		client.publish(topicArr, message);
	}
}

void turnOff() {
	Serial.printf("Turning off...\n");
  //digitalWrite(2,HIGH);
  irsend.sendRaw(rawOff,162,38);
	sendToBroker("report/powerState", "OFF");
  //digitalWrite(2,LOW);

}

void turnOn() {
	Serial.printf("Turning on...\n");
  //digitalWrite(2,HIGH);
  irsend.sendRaw(rawOn,162,38);
	sendToBroker("report/powerState", "ON");
  //digitalWrite(2,LOW);
}

//void codeForTask1( void * parameter ) {
//  for(;;){
//	client.loop();
//vTaskDelay(1);
//    }
///* vTaskDelete(NULL); */
//  }
//
//void codeForTask2( void * parameter ) {
//  for(;;){
//	checkMqtt();
//vTaskDelay(1);
//    }
///* vTaskDelete(NULL); */
//  }
//
//TaskHandle_t Task1, Task2;

void setup() {
	Serial.begin(115200);
  //pinMode(2,OUTPUT);
	startWifi();
	startMqtt();
//xTaskCreatePinnedToCore(codeForTask1,"clientTask",100000,NULL,1,&Task1,1);
//xTaskCreatePinnedToCore(codeForTask2,"checkMqtt",100000,NULL,1,&Task2,0);
}
//void loop (){}
 void loop() { 
     client.loop();
     checkMqtt(); 
   } 
