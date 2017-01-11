/**
* @file TestClient.cpp
*
* App for testing the Cayenne MQTT C++ library functionality.
*/


#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include "MQTTLinux.h"
#include "CayenneMQTTClient.h"

bool checkMessages = false;
bool messageMatched = false;
bool messageReceived = false;
MQTTNetwork ipstack;
CayenneMQTT::MQTTClient<MQTTNetwork, MQTTTimer> mqttClient(ipstack);
CayenneMQTT::MessageData testMessage;
size_t failureCount = 0;

/**
* Output usage info for this test app.
*/
void usage(void)
{
	printf("Cayenne MQTT Test\n");
	printf("Usage: testclient <options>, where options are:\n");
	printf("  --host <hostname> (default is %s)\n", CAYENNE_DOMAIN);
	printf("  --port <port> (default is %d)\n", CAYENNE_PORT);
	printf("  --username <username> (default is username)\n");
	printf("  --password <password> (default is password)\n");
	printf("  --clientID <clientID> (default is clientID)\n");
	printf("  --help (show this)\n");
	exit(-1);
}


struct opts_struct
{
	char* username;
	char* password;
	char* clientID;
	char* host;
	int port;
} opts =
{
	(char*)"username", (char*)"password", (char*)"clientID", (char*)CAYENNE_DOMAIN, CAYENNE_PORT
};

char alternateClientID[] = "alternateClientID";

/**
* Get options from the command line.
* @param[in] argc Count of command line arguments.
* @param[in] argv Command line argument string array.
*/
void getOptions(int argc, char** argv)
{
	int count = 1;

	while (count < argc)
	{
		if (strcmp(argv[count], "--help") == 0)
		{
			usage();
		}
		else if (strcmp(argv[count], "--host") == 0)
		{
			if (++count < argc)
				opts.host = argv[count];
			else
				usage();
		}
		else if (strcmp(argv[count], "--port") == 0)
		{
			if (++count < argc)
				opts.port = atoi(argv[count]);
			else
				usage();
		}
		else if (strcmp(argv[count], "--username") == 0)
		{
			if (++count < argc)
				opts.username = argv[count];
			else
				usage();
		}
		else if (strcmp(argv[count], "--password") == 0)
		{
			if (++count < argc)
				opts.password = argv[count];
			else
				usage();
		}
		else if (strcmp(argv[count], "--clientID") == 0)
		{
			if (++count < argc)
				opts.clientID = argv[count];
			else
				usage();
		}
		count++;
	}
	
}

/**
* Print message data.
* @param[in] message The message to print
*/
void outputMessage(CayenneMQTT::MessageData& message)
{
	printf("topic=%d channel=%d", message.topic, message.channel);
	if (message.clientID) {
		printf(" clientID=%s", message.clientID);
	}
	if (message.type) {
		printf(" type=%s", message.type);
	}
	for (size_t i = 0; i < message.valueCount; ++i) {
		if (message.getUnit(i)) {
			printf(" unit=%s", message.getUnit(i));
		}
		if (message.getValue(i)) {
			printf(" value=%s", message.getValue(i));
		}
	}
	if (message.id) {
		printf(" id=%s", message.id);
	}
}

/**
* Check if a message matches the sent test message.
* @param[in] message The message to check against the test message
*/
void checkMessage(CayenneMQTT::MessageData& message)
{
	outputMessage(message);
	messageMatched = true;
	if (message.topic != testMessage.topic) {
		printf(" top err: %u\n", testMessage.topic);
		messageMatched = false;
	}
	if (message.channel != testMessage.channel) {
		printf(" chan err: %u\n", testMessage.channel);
		messageMatched = false;
	}
	if (((message.clientID != NULL) || (testMessage.clientID != NULL)) && (message.clientID && testMessage.clientID && strcmp(message.clientID, testMessage.clientID) != 0)) {
		printf(" devID err: %s\n", testMessage.clientID ? testMessage.clientID : "NULL");
		messageMatched = false;
	}
	if (((message.type != NULL) || (testMessage.type != NULL)) && (message.type && testMessage.type && strcmp(message.type, testMessage.type) != 0)) {
		printf(" type err: %s\n", testMessage.type ? testMessage.type : "NULL");
		messageMatched = false;
	}
	if (((message.id != NULL) || (testMessage.id != NULL)) && (message.id && testMessage.id && strcmp(message.id, testMessage.id) != 0)) {
		printf(" id err: %s\n", testMessage.id ? testMessage.id : "NULL");
		messageMatched = false;
	}
	if (message.valueCount != testMessage.valueCount) {
		printf(" valCount err: %u\n", testMessage.valueCount);
		messageMatched = false;
	}
	for (size_t i = 0; i < message.valueCount; ++i) {
		if (((message.values[i].value != NULL) || (testMessage.values[i].value != NULL)) &&
			(message.values[i].value && testMessage.values[i].value && (strcmp(message.values[i].value, testMessage.values[i].value) != 0))) {
			printf(" val%d err: %s\n", i, testMessage.values[i].value ? testMessage.values[i].value : "NULL");
			messageMatched = false;
		}
		if (((message.values[i].unit != NULL) || (testMessage.values[i].unit != NULL)) &&
			(message.values[i].unit && testMessage.values[i].unit && (strcmp(message.values[i].unit, testMessage.values[i].unit) != 0))) {
			printf(" unit%d err: %s\n", i, testMessage.values[i].unit ? testMessage.values[i].unit : "NULL");
			messageMatched = false;
		}
	}
}

/**
* Default message handler for messages.
* @param[in] message The message received from the Cayenne server
*/
void defaultMessageHandler(CayenneMQTT::MessageData& message)
{
	if (!checkMessages)
		return;
	printf(" -> defaultMessageHandler: ");
	checkMessage(message);
	messageReceived = true;
}

/**
* Message handler for command messages.
* @param[in] message The message received from the Cayenne server
*/
void commandMessageHandler(CayenneMQTT::MessageData& message)
{
	if (!checkMessages)
		return;
	printf(" -> commandMessageHandler: ");
	checkMessage(message);
	if (message.topic == COMMAND_TOPIC && message.id)
	{
		int rc = CAYENNE_SUCCESS;
		if ((message.channel == 0) || (message.channel == 1)) {
			switch (message.channel) {
			case 0:
				rc = mqttClient.publishResponse(message.id, "error message", message.clientID);
				break;
			case 1:
				rc = mqttClient.publishResponse(message.id, NULL, message.clientID);
				break;
			}
			if (rc == CAYENNE_SUCCESS) {
				printf(" - Responded, rc: %d", rc);
			}
			else {
				failureCount++;
				printf(" - Response FAILURE, rc: %d", rc);
			}
		}
	}
	messageReceived = true;
}

/**
* Subscribe to a topic.
* @param[in] topic Cayenne topic
* @param[in] channel The topic channel, CAYENNE_NO_CHANNEL for none, CAYENNE_ALL_CHANNELS for all
* @param[in] handler The message handler to use for this topic, NULL to use the default handler
* @param[in] clientID The client ID to use in the topic, NULL to use the clientID the client was initialized with
*/
void subscribe(CayenneTopic topic, unsigned int channel, CayenneMQTT::MQTTClient<MQTTNetwork, MQTTTimer>::CayenneMessageHandler handler, const char* clientID)
{
	printf("Subscribe: topic=%d ", topic);
	int rc = mqttClient.subscribe(topic, channel, handler, clientID);
	char buffer[CAYENNE_MAX_MESSAGE_SIZE] = { 0 };
	CayenneBuildTopic(buffer, sizeof(buffer), opts.username, clientID ? clientID : opts.clientID, topic, channel);
	printf(buffer);
	printf(", rc: %d\n", rc);
	if (rc != CAYENNE_SUCCESS)
		failureCount++;
}

/**
* Unsubscribe from a topic.
* @param[in] topic Cayenne topic
* @param[in] channel The topic channel, CAYENNE_NO_CHANNEL for none, CAYENNE_ALL_CHANNELS for all
* @param[in] clientID The client ID to use in the topic, NULL to use the clientID the client was initialized with
*/
void unsubscribe(CayenneTopic topic, unsigned int channel, const char* clientID)
{
	printf("Unsubscribe: topic=%d ", topic);
	int rc = mqttClient.unsubscribe(topic, channel, clientID);
	char buffer[CAYENNE_MAX_MESSAGE_SIZE] = { 0 };
	CayenneBuildTopic(buffer, sizeof(buffer), opts.username, clientID ? clientID : opts.clientID, topic, channel);
	printf(buffer);
	printf(", rc: %d\n", rc);
	if (rc != CAYENNE_SUCCESS)
		failureCount++;
}

/**
* Connect to the server.
* @return Error code
*/
int connectClient(void)
{
	printf("Connecting to %s:%d, username: %s, password: %s, client ID: %s\n", opts.host, opts.port, opts.username, opts.password, opts.clientID);

	int rc = 0;
	while ((rc = ipstack.connect(opts.host, opts.port)) != 0)
	{
		printf("TCP connect failed, rc: %d\n", rc);
		sleep(2);
	}

	printf("MQTT connecting\n");
	rc = mqttClient.connect();
	if (rc != 0) {
		printf("MQTT connect failed, rc: %d", rc);
		switch(rc)
		{
		case 1:
			printf(" - unacceptable protocol version");
			break;
		case 2:
			printf(" - identifier rejected");
			break;
		case 3:
			printf(" - server unavailable");
			break;
		case 4:
			printf(" - bad username or password");
			break;
		case 5:
			printf(" - not authorized, check username and password");
			break;
		}
		printf("\n");
		return rc;
	}
	else {
		printf("MQTT connected\n");
	}
	subscribe(COMMAND_TOPIC, CAYENNE_ALL_CHANNELS, commandMessageHandler, NULL);
	subscribe(COMMAND_TOPIC, CAYENNE_ALL_CHANNELS, commandMessageHandler, alternateClientID);
	subscribe(DATA_TOPIC, CAYENNE_ALL_CHANNELS, NULL, NULL);
	subscribe(SYS_MODEL_TOPIC, CAYENNE_NO_CHANNEL, NULL, NULL);
	subscribe(SYS_VERSION_TOPIC, CAYENNE_NO_CHANNEL, NULL, NULL);

	return CAYENNE_SUCCESS;
}

/**
* Wait to receive a message from the server.
* @return true if a message was received, false otherwise
*/
bool waitForMessage(void)
{
	MQTTTimer timer(1000);
	messageReceived = false;
	while (!messageReceived && !timer.expired())
	{
		mqttClient.yield(50);
		if (!ipstack.connected() || !mqttClient.connected())
		{
			ipstack.disconnect();
			mqttClient.disconnect();
			printf("Reconnecting\n");
			connectClient();
			break;
		}
	}
	return messageReceived;
}

/**
* Initialize a test message struct to compare against the received one.
* @param[in] topic Cayenne topic
* @param[in] channel The channel to send data to, or CAYENNE_NO_CHANNEL if there is none
* @param[in] value Data value
* @param[in] type Data type, can be NULL
* @param[in] unit Data unit, can be NULL
* @param[in] clientID The client ID to use in the topic, NULL to use the clientID the client was initialized with
*/
void initTestMessage(CayenneTopic topic, unsigned int channel, const char* value, const char* type, const char* unit, const char* clientID)
{
	testMessage.id = NULL;
	testMessage.topic = topic;
	testMessage.channel = channel;
	testMessage.clientID = clientID ? clientID : opts.clientID;
	testMessage.type = type;
	testMessage.values[0].value = value;
	testMessage.values[0].unit = unit;
	testMessage.valueCount = 1;
	printf("Publish: ");
	outputMessage(testMessage);
}

/**
* Check that a message was published successfully. If specified, this will check the sent message matches the received one.
* @param[in] topic Cayenne topic
* @param[in] type Data type, can be NULL
* @param[in] unit Data unit, can be NULL
* @param[in] shouldReceive This check should receive a message in one of the message handlers and make sure it matches the sent message
*/
void checkPublishSuccess(CayenneTopic topic, const char* type, const char* unit, bool shouldReceive)
{
	switch (topic)
	{
	//Some messages are parsed differently so update the test message to make the comparison work.
	case COMMAND_TOPIC:
		testMessage.id = type;
		testMessage.values[0].value = unit;
		testMessage.values[0].unit = NULL;
		testMessage.type = NULL;
		break;
	default:
		break;
	}
	waitForMessage();
	bool succeeded = false;
	if (messageReceived && shouldReceive && messageMatched) {
		succeeded = true;
	}
	else if (!messageReceived && !shouldReceive)
		succeeded = true;

	if (succeeded) {
		printf(" - SUCCESS\n");
	}
	else {
		failureCount++;
		printf(" - FAILURE\n");
	}
}

/**
* Test publishing a string value.
* @param[in] topic Cayenne topic
* @param[in] channel The channel to send data to, or CAYENNE_NO_CHANNEL if there is none
* @param[in] value Data value
* @param[in] type Data type, can be NULL
* @param[in] unit Data unit, can be NULL
* @param[in] shouldReceive This test should receive a message in one of the message handlers
* @param[in] clientID The client ID to use in the topic, NULL to use the clientID the client was initialized with
*/
void testPublish(CayenneTopic topic, unsigned int channel, const char* value, const char* type, const char* unit, bool shouldReceive, const char* clientID)
{
	initTestMessage(topic, channel, value, type, unit, clientID);
	int rc = CAYENNE_SUCCESS;
	if ((rc = mqttClient.publishData(topic, channel, type, unit, value, clientID)) != CAYENNE_SUCCESS) {
		failureCount++;
		printf(" - Publish FAILURE, rc: %d\n", rc);
		return;
	}
	checkPublishSuccess(topic, type, unit, shouldReceive);
}

/**
* Test publishing an int value.
* @param[in] topic Cayenne topic
* @param[in] channel The channel to send data to, or CAYENNE_NO_CHANNEL if there is none
* @param[in] value Data value
* @param[in] type Data type, can be NULL
* @param[in] unit Data unit, can be NULL
* @param[in] shouldReceive This test should receive a message in one of the message handlers
* @param[in] clientID The client ID to use in the topic, NULL to use the clientID the client was initialized with
*/
void testPublishInt(CayenneTopic topic, unsigned int channel, int value, const char* type, const char* unit, bool shouldReceive, const char* clientID)
{
	char str[2 + 8 * sizeof(value)];
	snprintf(str, sizeof(str), "%d", value);
	initTestMessage(topic, channel, str, type, unit, clientID);
	int rc = CAYENNE_SUCCESS;
	if ((rc = mqttClient.publishData(topic, channel, type, unit, value, clientID)) != CAYENNE_SUCCESS) {
		failureCount++;
		printf(" - Publish FAILURE, rc: %d\n", rc);
		return;
	}
	checkPublishSuccess(topic, type, unit, shouldReceive);
}

/**
* Test publishing an unsigned int value.
* @param[in] topic Cayenne topic
* @param[in] channel The channel to send data to, or CAYENNE_NO_CHANNEL if there is none
* @param[in] value Data value
* @param[in] type Data type, can be NULL
* @param[in] unit Data unit, can be NULL
* @param[in] shouldReceive This test should receive a message in one of the message handlers
* @param[in] clientID The client ID to use in the topic, NULL to use the clientID the client was initialized with
*/
void testPublishUInt(CayenneTopic topic, unsigned int channel, unsigned int value, const char* type, const char* unit, bool shouldReceive, const char* clientID)
{
	char str[1 + 8 * sizeof(value)];
	snprintf(str, sizeof(str), "%u", value);
	initTestMessage(topic, channel, str, type, unit, clientID);
	int rc = CAYENNE_SUCCESS;
	if ((rc = mqttClient.publishData(topic, channel, type, unit, value, clientID)) != CAYENNE_SUCCESS) {
		failureCount++;
		printf(" - Publish FAILURE, rc: %d\n", rc);
		return;
	}
	checkPublishSuccess(topic, type, unit, shouldReceive);
}

/**
* Test publishing a long value.
* @param[in] topic Cayenne topic
* @param[in] channel The channel to send data to, or CAYENNE_NO_CHANNEL if there is none
* @param[in] value Data value
* @param[in] type Data type, can be NULL
* @param[in] unit Data unit, can be NULL
* @param[in] shouldReceive This test should receive a message in one of the message handlers
* @param[in] clientID The client ID to use in the topic, NULL to use the clientID the client was initialized with
*/
void testPublishLong(CayenneTopic topic, unsigned int channel, long value, const char* type, const char* unit, bool shouldReceive, const char* clientID)
{
	char str[2 + 8 * sizeof(value)];
	snprintf(str, sizeof(str), "%ld", value);
	initTestMessage(topic, channel, str, type, unit, clientID);
	int rc = CAYENNE_SUCCESS;
	if ((rc = mqttClient.publishData(topic, channel, type, unit, value, clientID)) != CAYENNE_SUCCESS) {
		failureCount++;
		printf(" - Publish FAILURE, rc: %d\n", rc);
		return;
	}
	checkPublishSuccess(topic, type, unit, shouldReceive);
}

/**
* Test publishing an unsigned long value.
* @param[in] topic Cayenne topic
* @param[in] channel The channel to send data to, or CAYENNE_NO_CHANNEL if there is none
* @param[in] value Data value
* @param[in] type Data type, can be NULL
* @param[in] unit Data unit, can be NULL
* @param[in] shouldReceive This test should receive a message in one of the message handlers
* @param[in] clientID The client ID to use in the topic, NULL to use the clientID the client was initialized with
*/
void testPublishULong(CayenneTopic topic, unsigned int channel, unsigned long value, const char* type, const char* unit, bool shouldReceive, const char* clientID)
{
	char str[2 + 8 * sizeof(value)];
	snprintf(str, sizeof(str), "%lu", value);
	initTestMessage(topic, channel, str, type, unit, clientID);
	int rc = CAYENNE_SUCCESS;
	if ((rc = mqttClient.publishData(topic, channel, type, unit, value, clientID)) != CAYENNE_SUCCESS) {
		failureCount++;
		printf(" - Publish FAILURE, rc: %d\n", rc);
		return;
	}
	checkPublishSuccess(topic, type, unit, shouldReceive);
}

/**
* Test publishing a double value.
* @param[in] topic Cayenne topic
* @param[in] channel The channel to send data to, or CAYENNE_NO_CHANNEL if there is none
* @param[in] value Data value
* @param[in] type Data type, can be NULL
* @param[in] unit Data unit, can be NULL
* @param[in] shouldReceive This test should receive a message in one of the message handlers
* @param[in] clientID The client ID to use in the topic, NULL to use the clientID the client was initialized with
*/
void testPublishDouble(CayenneTopic topic, unsigned int channel, double value, const char* type, const char* unit, bool shouldReceive, const char* clientID)
{
	char str[33];
	snprintf(str, 33, "%2.3f", value);
	initTestMessage(topic, channel, str, type, unit, clientID);
	int rc = CAYENNE_SUCCESS;
	if ((rc = mqttClient.publishData(topic, channel, type, unit, value, clientID)) != CAYENNE_SUCCESS) {
		failureCount++;
		printf(" - Publish FAILURE, rc: %d\n", rc);
		return;
	}
	checkPublishSuccess(topic, type, unit, shouldReceive);
}

/**
* Test publishing a float value.
* @param[in] topic Cayenne topic
* @param[in] channel The channel to send data to, or CAYENNE_NO_CHANNEL if there is none
* @param[in] value Data value
* @param[in] type Data type, can be NULL
* @param[in] unit Data unit, can be NULL
* @param[in] shouldReceive This test should receive a message in one of the message handlers
* @param[in] clientID The client ID to use in the topic, NULL to use the clientID the client was initialized with
*/
void testPublishFloat(CayenneTopic topic, unsigned int channel, float value, const char* type, const char* unit, bool shouldReceive, const char* clientID)
{
	char str[33];
	snprintf(str, 33, "%2.3f", value);
	initTestMessage(topic, channel, str, type, unit, clientID);
	int rc = CAYENNE_SUCCESS;
	if ((rc = mqttClient.publishData(topic, channel, type, unit, value, clientID)) != CAYENNE_SUCCESS) {
		failureCount++;
		printf(" - Publish FAILURE, rc: %d\n", rc);
		return;
	}
	checkPublishSuccess(topic, type, unit, shouldReceive);
}

/**
* Main function.
* @param[in] argc Count of command line arguments.
* @param[in] argv Command line argument string array.
* @return Failure count, 0 if no tests failed
*/
int main(int argc, char** argv)
{
#ifdef PARSE_INFO_PAYLOADS // Defined by the makefile so we can receive and check DATA_TOPIC messages.
	bool parseInfoPayload = true;
#else
	bool parseInfoPayload = false;
#endif

	struct timeval before, after, elapsed;
	gettimeofday(&before, NULL);

	getOptions(argc, argv);

	printf("Cayenne MQTT Test\n");
	mqttClient.init(opts.username, opts.password, opts.clientID);
	mqttClient.setDefaultMessageHandler(defaultMessageHandler);
	if (connectClient() != CAYENNE_SUCCESS) {
		printf("Connection failed, exiting\n");
		if (mqttClient.connected())
			mqttClient.disconnect();
		if (ipstack.connected())
			ipstack.disconnect();
		return 1;
	}
	//Wait for any retained messages so they don't mess up the tests.
	waitForMessage();

	printf("Test Publish\n");
	checkMessages = true;
	testPublish(DATA_TOPIC, 1, "1", TYPE_TEMPERATURE, UNIT_CELSIUS, parseInfoPayload, NULL);
	testPublishInt(DATA_TOPIC, 2, -2, TYPE_TEMPERATURE, UNIT_KELVIN, parseInfoPayload, NULL);
	testPublishUInt(DATA_TOPIC, 3, 3, TYPE_TEMPERATURE, UNIT_FAHRENHEIT, parseInfoPayload, NULL);
	testPublishLong(DATA_TOPIC, 4, -4, TYPE_PROXIMITY, UNIT_CENTIMETER, parseInfoPayload, NULL);
	testPublishULong(DATA_TOPIC, 5, 5, TYPE_LUMINOSITY, UNIT_LUX, parseInfoPayload, NULL);
	testPublishDouble(DATA_TOPIC, 6, 6.6, TYPE_BAROMETRIC_PRESSURE, UNIT_HECTOPASCAL, parseInfoPayload, NULL);
	testPublishFloat(DATA_TOPIC, 7, 7.7, TYPE_RELATIVE_HUMIDITY, UNIT_PERCENT, parseInfoPayload, NULL);
	testPublish(SYS_MODEL_TOPIC, CAYENNE_NO_CHANNEL, "Model", NULL, NULL, parseInfoPayload, NULL);
	testPublish(SYS_VERSION_TOPIC, CAYENNE_NO_CHANNEL, "1.0.0", NULL, NULL, parseInfoPayload, NULL);
	testPublish(COMMAND_TOPIC, 0, NULL, "1", "respond with error", true, NULL);
	testPublish(COMMAND_TOPIC, 1, NULL, "2", "respond with ok", true, NULL);
	testPublish(COMMAND_TOPIC, 2, NULL, "1", "alternate clientID", true, alternateClientID);

	unsubscribe(COMMAND_TOPIC, CAYENNE_ALL_CHANNELS, NULL);
	unsubscribe(COMMAND_TOPIC, CAYENNE_ALL_CHANNELS, alternateClientID);
	unsubscribe(DATA_TOPIC, CAYENNE_ALL_CHANNELS, NULL);
	unsubscribe(SYS_MODEL_TOPIC, CAYENNE_NO_CHANNEL, NULL);
	unsubscribe(SYS_VERSION_TOPIC, CAYENNE_NO_CHANNEL, NULL);

	if (mqttClient.connected())
		mqttClient.disconnect();
	if (ipstack.connected())
		ipstack.disconnect();

	gettimeofday(&after, NULL);
	timersub(&after, &before, &elapsed);
	printf("Cayenne MQTT Test finished, elapsed time: %ld ms, failure count: %d\n", (elapsed.tv_sec < 0) ? 0 : elapsed.tv_sec * 1000 + elapsed.tv_usec / 1000, failureCount);

	return failureCount;
}


