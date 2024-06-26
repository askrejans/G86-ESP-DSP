#ifndef MQTT_SETUP_H
#define MQTT_SETUP_H

#include <Arduino.h>
#include <MQTT.h>
#include <WiFiClient.h>
#include "WiFiSetup.h"

/**
 * @brief Global constants for MQTT client names and topics.
 */
extern const char *PRIMARY_MQTT_CLIENT_NAME;
extern const char *SECONDARY_MQTT_CLIENT_NAME;
extern const char *MQTT_TOPIC_BASE;

/**
 * @brief Global instances of WiFiSetup, message flags, and message buffers.
 */
extern WiFiSetup wifiSetup;
extern bool newMessageAvailable;
extern char newMessage[128];
extern volatile bool newMessageAvailable2;
extern volatile char newMessage2[128];

/**
 * @brief Class for setting up and managing MQTT communication.
 */
class MqttSetup
{
public:
    /**
     * @brief Callback function for receiving MQTT messages on the primary channel.
     * @param topic The MQTT topic.
     * @param payload The message payload.
     */
    static void MqttMessageReceivedPrimary(String &topic, String &payload);

    /**
     * @brief Callback function for receiving MQTT messages on the secondary channel.
     * @param topic The MQTT topic.
     * @param payload The message payload.
     */
    static void MqttMessageReceivedSecondary(String &topic, String &payload);


    /**
     * @brief Initializes the MQTT setup.
     */
    void begin();

    /**
     * @brief Sets up the MQTT connection.
     */
    void setupMqtt();

    /**
     * @brief Connects to the MQTT broker.
     */
    void connect();

    /**
     * @brief Gets the MQTT client for primary channel.
     * @return Reference to the MQTT client.
     */
    MQTTClient &getMqttClient();

    /**
     * @brief Gets the MQTT client for secondary channel.
     * @return Reference to the MQTT client.
     */
    MQTTClient mqtt;
    MQTTClient mqtt2;

private:
    WiFiClient net;
    WiFiClient net2;

    /**
     * @brief Static variables for managing LED blinking.
     */
    static unsigned long lastBlinkMillis;
    static bool colonVisible;
};

#endif // MQTT_SETUP_H
