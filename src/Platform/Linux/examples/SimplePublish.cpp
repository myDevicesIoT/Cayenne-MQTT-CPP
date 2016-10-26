/**
* @file SimplePublish.cpp
*
* Simplified example app for using the Cayenne MQTT C++ library to publish example data.
*/

#include "MQTTLinux.h"
#include "CayenneMQTTClient.h"

// Cayenne authentication info. This should be obtained from the Cayenne Dashboard.
char username[] = "MQTT_USERNAME";
char password[] = "MQTT_PASSWORD";
char clientID[] = "CLIENT_ID";

MQTTNetwork ipstack;
CayenneMQTT::MQTTClient<MQTTNetwork, MQTTTimer> mqttClient(ipstack, username, password, clientID);


// Connect to the Cayenne server.
int connectClient(void)
{
	// Connect to the server.
	int error = 0;
	printf("Connecting to %s:%d\n", CAYENNE_DOMAIN, CAYENNE_PORT);
	if ((error = ipstack.connect(CAYENNE_DOMAIN, CAYENNE_PORT)) != 0) {
		return error;
	}

	if ((error = mqttClient.connect()) != MQTT::SUCCESS) {
		ipstack.disconnect();
		return error;
	}
	printf("Connected\n");

	// Send device info. Here we just send some example values for the system info. These should be changed to use actual system data, or removed if not needed.
	mqttClient.publishData(SYS_VERSION_TOPIC, CAYENNE_NO_CHANNEL, NULL, NULL, CAYENNE_VERSION);
	mqttClient.publishData(SYS_MODEL_TOPIC, CAYENNE_NO_CHANNEL, NULL, NULL, "Linux");

	return CAYENNE_SUCCESS;
}


// Main loop where MQTT code is run.
void loop(void)
{
	while (1) {
		// Yield to allow MQTT message processing.
		mqttClient.yield(1000);

		// Publish some example data every second. This should be changed to send your actual data to Cayenne.
		mqttClient.publishData(DATA_TOPIC, 0, TEMPERATURE, CELSIUS, 30.5);
		mqttClient.publishData(DATA_TOPIC, 1, LUMINOSITY, LUX, 1000);
	}
}

// Main function.
int main(int argc, char** argv)
{
	// Connect to Cayenne.
	if (connectClient() == CAYENNE_SUCCESS) {
		// Run main loop.
		loop();
	}
	else {
		printf("Connection failed, exiting\n");
	}

	return 0;
}


