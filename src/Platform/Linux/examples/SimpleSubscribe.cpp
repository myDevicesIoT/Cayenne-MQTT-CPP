/**
* @file SimpleSubscribe.cpp
*
* Simplified example app for using the Cayenne MQTT C++ library to receive example data.
*/

#include "MQTTLinux.h"
#include "CayenneMQTTClient.h"

// Cayenne authentication info. This should be obtained from the Cayenne Dashboard.
char username[] = "MQTT_USERNAME";
char password[] = "MQTT_PASSWORD";
char clientID[] = "CLIENT_ID";

MQTTNetwork ipstack;
CayenneMQTT::MQTTClient<MQTTNetwork, MQTTTimer> mqttClient(ipstack, username, password, clientID);


// Handle messages received from the Cayenne server.
void messageArrived(CayenneMQTT::MessageData& message)
{
	printf("Message received on channel %d\n", message.channel);
	
	// Add code to process the message here.
	if (message.topic == COMMAND_TOPIC) {
		// If this is a command message we publish a response to show we recieved it. Here we are just sending a default 'OK' response.
		// An error response should be sent if there are issues processing the message.
		mqttClient.publishResponse(message.id, NULL, message.clientID);

		// Send the updated state for the channel so it is reflected in the Cayenne dashboard. If a command is successfully processed
		// the updated state will usually just be the value received in the command message.
		mqttClient.publishData(DATA_TOPIC, message.channel, NULL, NULL, message.getValue());
	}
}

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

	// Subscribe to required topics. Here we subscribe to the Command and Config topics.
	mqttClient.subscribe(COMMAND_TOPIC, CAYENNE_ALL_CHANNELS);
	mqttClient.subscribe(CONFIG_TOPIC, CAYENNE_ALL_CHANNELS);

	return CAYENNE_SUCCESS;
}


// Main loop where MQTT code is run.
void loop(void)
{
	while (1) {
		// Yield to allow MQTT message processing.
		mqttClient.yield(1000);
	}
}

// Main function.
int main(int argc, char** argv)
{
	// Set the default function that receives Cayenne messages.
	mqttClient.setDefaultMessageHandler(messageArrived);

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


