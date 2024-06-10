#include "MqttSetup.h"

unsigned long MqttSetup::lastBlinkMillis = 0;
bool MqttSetup::colonVisible = true;

/**
 * Initialize MQTT setup.
 */
void MqttSetup::begin()
{
    // Initialize MQTT connection for Primary client
    mqtt.begin(wifiSetup.config.mqtt_server, net);
    mqtt.onMessage(MqttMessageReceivedPrimary);

    Serial.print("\nconnecting to MQTT Primary to " + String(wifiSetup.config.mqtt_server) + ":" + String(wifiSetup.config.mqtt_port));

    // Attempt to connect to MQTT Primary
    while (!mqtt.connect(PRIMARY_MQTT_CLIENT_NAME, "public", "public"))
    {
        Serial.print(".");
        delay(1000);
    }

    Serial.println("\nMQTT Primary connected!");

    // Initialize MQTT connection for Secondary client
    mqtt2.begin(wifiSetup.config.mqtt_server, net2);
    mqtt2.onMessage(MqttMessageReceivedSecondary);

    Serial.print("\nconnecting to MQTT Secondary to " + String(wifiSetup.config.mqtt_server) + ":" + String(wifiSetup.config.mqtt_port));

    // Attempt to connect to MQTT Secondary
    while (!mqtt2.connect(SECONDARY_MQTT_CLIENT_NAME, "public", "public"))
    {
        Serial.print(".");
        delay(1000);
    }

    Serial.println("\nMQTT Secondary connected!");
}

/**
 * Handle MQTT connections for both Primary and Secondary clients.
 */
void MqttSetup::connect()
{
    mqtt.loop();
    mqtt2.loop();
}

/**
 * Callback function for receiving MQTT messages on the Primary channel.
 * @param topic The MQTT topic.
 * @param payload The MQTT payload.
 */
void MqttSetup::MqttMessageReceivedPrimary(String &topic, String &payload)
{

strncpy(newMessage, payload.c_str(), sizeof(newMessage) - 1);

    //Serial.print("Received RPM: ");
    //Serial.println(newMessage);

}

/**
 * Callback function for receiving MQTT messages on the Secondary channel.
 * @param topic The MQTT topic.
 * @param payload The MQTT payload.
 */
void MqttSetup::MqttMessageReceivedSecondary(String &topic, String &payload)
{
  
}