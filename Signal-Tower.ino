/*
  Project:      signal_tower
  Description:  controls Patlite signal tower via MQTT

  See README.md for target information
*/

// hardware and internet configuration parameters
#include "config.h"
// private credentials for network, MQTT
#include "secrets.h"

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ESP32)
  #include <WiFi.h>
#endif
// MQTT uses WiFiClient class to create TCP connections
WiFiClient client;

#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>

/*
 * Home Assistant YAML configuration for each light
  light:
    - name: "Signal Tower Red"
      object_id: "signaltower_01_red"
      state_topic: "site/signaltower/red/status"
      command_topic: "site/signaltower/red/switch"
      unique_id: "GENERATE-UNIQUE-UUID-HERE"
      optimistic: false

    - name: "Signal Tower orange"
      object_id: "signaltower_01_orange"
      state_topic: "site/signaltower/orange/status"
      command_topic: "site/signaltower/orange/switch"
      unique_id: "GENERATE-UNIQUE-UUID-HERE"
      optimistic: false

    - name: "Signal Tower Green"
      object_id: "signaltower_01_green"
      state_topic: "site/signaltower/green/status"
      command_topic: "site/signaltower/green/switch"
      unique_id: "GENERATE-UNIQUE-UUID-HERE"
      optimistic: false

    - name: "Signal Tower Blue"
      object_id: "signaltower_01_blue"
      state_topic: "site/signaltower/blue/status"
      command_topic: "site/signaltower/blue/switch"
      unique_id: "GENERATE-UNIQUE-UUID-HERE"
      optimistic: false
*/

// hardware status data
typedef struct hdweData
{
  // float batteryPercent;
  // float batteryVoltage;
  // float batteryTemperatureF;
  int rssi;
} hdweData;
hdweData hardwareData;

// Create the MQTT client object used for publish and subscribe, binding it to the
// appropriate MQTT broker as defined in secrets.h
Adafruit_MQTT_Client ha_mqtt(&client, MQTT_BROKER, MQTT_PORT, DEVICE_ID, MQTT_USER, MQTT_PASS);
Adafruit_MQTT_Publish   towerPublish   = Adafruit_MQTT_Publish(&ha_mqtt,PUBLISH_TOPIC); 
Adafruit_MQTT_Subscribe towerCommand   = Adafruit_MQTT_Subscribe(&ha_mqtt,SUBSCRIBE_TOPIC);
Adafruit_MQTT_Subscribe redCommand     = Adafruit_MQTT_Subscribe(&ha_mqtt,RED_COMMAND_TOPIC);
Adafruit_MQTT_Publish   redStatePub    = Adafruit_MQTT_Publish(&ha_mqtt,RED_STATE_TOPIC);
Adafruit_MQTT_Subscribe greenCommand   = Adafruit_MQTT_Subscribe(&ha_mqtt,GREEN_COMMAND_TOPIC);
Adafruit_MQTT_Publish   greenStatePub  = Adafruit_MQTT_Publish(&ha_mqtt,GREEN_STATE_TOPIC);
Adafruit_MQTT_Subscribe blueCommand    = Adafruit_MQTT_Subscribe(&ha_mqtt,BLUE_COMMAND_TOPIC);
Adafruit_MQTT_Publish   blueStatePub   = Adafruit_MQTT_Publish(&ha_mqtt,BLUE_STATE_TOPIC);
Adafruit_MQTT_Subscribe orangeCommand  = Adafruit_MQTT_Subscribe(&ha_mqtt,ORANGE_COMMAND_TOPIC);
Adafruit_MQTT_Publish   orangeStatePub = Adafruit_MQTT_Publish(&ha_mqtt,ORANGE_STATE_TOPIC);

// Callback to be invoked when we the Command subscription receives a
// message.  Will be registered via the MQTT_Subscribe object on the
// appropriate topic.
// Command value passed in is an integer with the low order four bits indicating light
// status in accordance with the mask values defined in config.h.
void towercmdcallback(char *data, uint16_t len) {
  debugMessage(String("New tower command received, value (string) is: ")+data,2);

  // Set all lignts based on the command data value
  processCommand(atoi(data));

  // Publish the new light state info (for all four lights) using the same Command syntax
  debugMessage("Publishing new command",2);
  towerPublish.publish(String(tower_state).c_str());  // Use actual state, not just data received
}
void redcmdcallback(char *data, uint16_t len) {
  debugMessage(String("New red command received, value (string) is: ")+data,2);

  if(strcmp(data,LIGHT_ON) == 0) {
    redOn(true);  // And publish any status change
  }
  else if(strcmp(data,LIGHT_OFF) == 0) {
    redOff(true); // And publish any status change
  }
}

void greencmdcallback(char *data, uint16_t len) {
  debugMessage(String("New green command received, value (string) is: ")+data,2);

  if(strcmp(data,LIGHT_ON) == 0) {
    greenOn(true);  // And publish any status change
  }
  else if(strcmp(data,LIGHT_OFF) == 0) {
    greenOff(true);  // And publish any status change
  }
}

void bluecmdcallback(char *data, uint16_t len) {
  debugMessage(String("New blue command received, value (string) is: ")+data,2);

  if(strcmp(data,LIGHT_ON) == 0) {
    blueOn(true);  // And publish any status change
  }
  else if(strcmp(data,LIGHT_OFF) == 0) {
    blueOff(true);  // And publish any status change
  }
}

void orangecmdcallback(char *data, uint16_t len) {
  debugMessage(String("New orange command received, value (string) is: ")+data,2);

  if(strcmp(data,LIGHT_ON) == 0) {
    orangeOn(true);  // And publish any status change
  }
  else if(strcmp(data,LIGHT_OFF) == 0) {
    orangeOff(true);  // And publish any status change
  }
}

void setup() {

  // handle Serial first so debugMessage() works
  #ifdef DEBUG
    Serial.begin(115200);
    // wait for serial port connection
    while (!Serial);
  #endif

  debugMessage("signal tower started",1);
  debugMessage("Device ID: " + String(DEVICE_ID),2);
   
  // Enable LED output pins
  pinMode(LAMPPIN_RED,OUTPUT);
  pinMode(LAMPPIN_ORANGE,OUTPUT);
  pinMode(LAMPPIN_GREEN,OUTPUT);
  pinMode(LAMPPIN_BLUE,OUTPUT);

  networkConnect();

  // Enable command notification callback, and
  // subscribe to receive those notifications.  
  // Must configure before connecting to MQTT Broker.
  towerCommand.setCallback(towercmdcallback);   ha_mqtt.subscribe(&towerCommand);
  redCommand.setCallback(redcmdcallback);       ha_mqtt.subscribe(&redCommand);
  greenCommand.setCallback(greencmdcallback);   ha_mqtt.subscribe(&greenCommand);
  blueCommand.setCallback(bluecmdcallback);     ha_mqtt.subscribe(&blueCommand);
  orangeCommand.setCallback(orangecmdcallback); ha_mqtt.subscribe(&orangeCommand);

  mqttConnect();

  // Start up lamp sequence
  processCommand(0b0000);       delay(500); // All lights off
  processCommand(blue_mask);    delay(500);
  processCommand(green_mask);   delay(500);
  processCommand(orange_mask);  delay(500);
  processCommand(red_mask);     delay(500);
  processCommand(0b1111);  // All lights on
  
  // Report all lights on (start-up state)
  redStatePub.publish(LIGHT_ON);
  greenStatePub.publish(LIGHT_ON);
  blueStatePub.publish(LIGHT_ON);
  orangeStatePub.publish(LIGHT_ON);

  debugMessage("Now processing MQTT commands",1);
}

// uint32_t pubcnt = 0;

void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  mqttConnect();

  // Sit in a tight loop processing MQTT subscriptions for a specified number
  // of milliseconds, calling callbacks for any received.
  ha_mqtt.processPackets(10000);

  // // ping the server to keep the mqtt connection alive
  // // NOT required if you are publishing once every KEEPALIVE seconds
  // if(! ha_mqtt.ping()) {
  //   ha_mqtt.disconnect();
  // }
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void mqttConnect()
// Connects and reconnects to MQTT broker, call as needed to maintain connection
{
  // exit if already connected
  if (ha_mqtt.connected())
  {
    debugMessage(String("Already connected to MQTT broker ") + MQTT_BROKER,2);
    return;
  }

  int8_t mqttErr;

  for(int tries = 1; tries <= CONNECT_ATTEMPT_LIMIT; tries++)
  {
    if ((mqttErr = ha_mqtt.connect()) == 0)
    {
      debugMessage(String("Connected to MQTT broker ") + MQTT_BROKER,1);
      return;
    }

    ha_mqtt.disconnect();
    debugMessage(String("MQTT connection attempt ") + tries + " of " + CONNECT_ATTEMPT_LIMIT + " failed with error msg: " + ha_mqtt.connectErrorString(mqttErr),1);
    delay(CONNECT_ATTEMPT_INTERVAL*1000);
  }
}

// Interpret a command for the entire tower (all four lights) received via MQTT
// Currently just turns lights on and off using low four bits, where
// 0 = off and 1 = on.
void processCommand(int command)
{
  debugMessage(String("processCommand is: ")+command,2);
  if(command & red_mask) {
    redOn(false); 
  }
  else {
    redOff(false);
  }
  if(command & orange_mask) {
    orangeOn(false);
  }
  else {
    orangeOff(false);
  }
  if(command & green_mask) {
    greenOn(false);
  }
  else {
    greenOff(false);
  }
  if(command & blue_mask) {
    blueOn(false);
  }
  else {
    blueOff(false);
  }
}

// Handle everything associated with turning the red light on
void redOn(bool publish)
{
  digitalWrite(LAMPPIN_RED,HIGH);  // Turn the light on  
  debugMessage("Red = ON",2);
  redStatePub.publish(LIGHT_ON);

  tower_state |= red_mask;   // Set red lamp indicator bit
  if(publish == true) towerPublish.publish(String(tower_state).c_str());
}
void redOff(bool publish)
{
  digitalWrite(LAMPPIN_RED,LOW);
  debugMessage("Red = OFF",2);
  redStatePub.publish(LIGHT_OFF);
  
  tower_state &= ~red_mask;  // Clear red lamp indicator bit
  if(publish == true) towerPublish.publish(String(tower_state).c_str());
}

void greenOn(bool publish)
{
  digitalWrite(LAMPPIN_GREEN,HIGH);
  debugMessage("Green = ON",2);
  greenStatePub.publish(LIGHT_ON);

  tower_state |= green_mask;  // Set green lamp indicator bit
  if(publish == true) towerPublish.publish(String(tower_state).c_str());
}

void greenOff(bool publish)
{
  digitalWrite(LAMPPIN_GREEN,LOW);
  debugMessage("Green = OFF",2);
  greenStatePub.publish(LIGHT_OFF);

  tower_state &= ~green_mask; // Clear green lamp indicator bit
  if(publish == true) towerPublish.publish(String(tower_state).c_str());
}

void blueOn(bool publish)
{
  digitalWrite(LAMPPIN_BLUE,HIGH);
  debugMessage("Blue = ON",2);
  blueStatePub.publish(LIGHT_ON);

  tower_state |= blue_mask;  // Set blue lamp indicator bit
  if(publish == true) towerPublish.publish(String(tower_state).c_str());
}

void blueOff(bool publish)
{
  digitalWrite(LAMPPIN_BLUE,LOW);
  debugMessage("Blue = OFF",2);
  blueStatePub.publish(LIGHT_OFF);

  tower_state &= ~blue_mask;  // Clear blue lamp indicator bit
  if(publish == true) towerPublish.publish(String(tower_state).c_str());
}

void orangeOn(bool publish)
{
  digitalWrite(LAMPPIN_ORANGE,HIGH);
  debugMessage("orange = ON",2);
  orangeStatePub.publish(LIGHT_ON);

  tower_state |= orange_mask;  // Set orange lamp indicator bit
  if(publish == true)towerPublish.publish(String(tower_state).c_str());
}

void orangeOff(bool publish)
{
  digitalWrite(LAMPPIN_ORANGE,LOW);
  debugMessage("orange = OFF",2);
  orangeStatePub.publish(LIGHT_OFF);

  tower_state &= ~orange_mask;  // Clear orange lamp indicator bit
  if(publish == true) towerPublish.publish(String(tower_state).c_str());
}

bool networkConnect()
{
  // Run only if using network data endpoints
  // set hostname has to come before WiFi.begin
  WiFi.hostname(DEVICE_ID);

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  for (int tries = 1; tries <= CONNECT_ATTEMPT_LIMIT; tries++)
  // Attempts WiFi connection, and if unsuccessful, re-attempts after CONNECT_ATTEMPT_INTERVAL second delay for CONNECT_ATTEMPT_LIMIT times
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      hardwareData.rssi = abs(WiFi.RSSI());
      debugMessage(String("WiFi IP address lease from ") + WIFI_SSID + " is " + WiFi.localIP().toString(),1);
      debugMessage(String("WiFi RSSI is: ") + hardwareData.rssi + " dBm",1);
      return true;
    }
    debugMessage(String("Connection attempt ") + tries + " of " + CONNECT_ATTEMPT_LIMIT + " to " + WIFI_SSID + " failed",1);
    // use of delay() OK as this is initialization code
    delay(CONNECT_ATTEMPT_INTERVAL * 1000); // convered into milliseconds
  }
  return false;
}

void testLamps()
{
  // cycle all lamps as test
  digitalWrite(LAMPPIN_RED, HIGH);
  debugMessage("Red lamp on",1);
  delay(2000);
  digitalWrite(LAMPPIN_RED, LOW);
  debugMessage("Red lamp off",1);
  digitalWrite(LAMPPIN_ORANGE, HIGH);
  debugMessage("Orange lamp on",1);
  delay(2000);
  digitalWrite(LAMPPIN_ORANGE, LOW);
    debugMessage("Orange lamp off",1);

    digitalWrite(LAMPPIN_GREEN, HIGH);
  debugMessage("Green lamp on",1);
  delay(2000);
  digitalWrite(LAMPPIN_GREEN, LOW);
  debugMessage("Green lamp off",1);
    digitalWrite(LAMPPIN_BLUE, HIGH);
  debugMessage("Blue lamp on",1);
  delay(2000);
  digitalWrite(LAMPPIN_BLUE, LOW);
  debugMessage("Blue lamp off",1);
}

void debugMessage(String messageText, int messageLevel)
// wraps Serial.println as #define conditional
{
  #ifdef DEBUG
    if (messageLevel <= DEBUG)
    {
      Serial.println(messageText);
      Serial.flush();  // Make sure the message gets output (before any sleeping...)
    }
  #endif
}