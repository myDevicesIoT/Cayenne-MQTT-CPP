/*
The MIT License(MIT)

Cayenne MQTT Client Library
Copyright (c) 2016 myDevices

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files(the "Software"), to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense, and / or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the following conditions :
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef _CAYENNEMQTTCLIENT_h
#define _CAYENNEMQTTCLIENT_h

#include <string.h>
#include "MQTTClient.h"
#include "../CayenneUtils/CayenneDefines.h"
#include "../CayenneUtils/CayenneUtils.h"
#include "../CayenneUtils/CayenneDataArray.h"

namespace CayenneMQTT
{
	/**
	* Cayenne message data passed to message handler functions.
	*/
	typedef struct MessageData
	{
		const char* clientID; /**< The client ID of the message. */
		CayenneTopic topic; /**< The topic the message was received on. */
		unsigned int channel; /**< The channel the message was received on. */
		const char* id; /**< The message ID, if it is a command message, otherwise NULL. */
		const char* type; /**< The type of data in the message, if it exists, otherwise NULL. */
		CayenneValuePair values[CAYENNE_MAX_MESSAGE_VALUES]; /**< The unit/value data pairs in the message. The units and values can be NULL. */
		size_t valueCount; /**< The count of items in the values array. */

		/**
		* Get value at specified index.
		* @param[in] index Index of value to retrieve, if none is specified it gets the first value.
		* @return Value at the specified index, can be NULL.
		*/
		const char* getValue(size_t index = 0) const { return values[index].value; }

		/**
		* Get unit at specified index.
		* @param[in] index Index of unit to retrieve, if none is specified it gets the first unit.
		* @return Unit at the specified index, can be NULL.
		*/
		const char* getUnit(size_t index = 0) const { return values[index].unit; }
	} MessageData;
	
	/**
	* Client class for connecting to Cayenne via MQTT.
	* @class MQTTClient
	* @param Network A network class with the methods: read, write. See NetworkInterface.h for function definitions.
	* @param Timer A timer class with the methods: countdown_ms, countdown, left_ms, expired. See TimerInterface.h for function definitions.
	* @param MAX_MQTT_PACKET_SIZE Maximum size of an MQTT message, in bytes.
	* @param MAX_MESSAGE_HANDLERS Maximum number of message handlers.
	*/
	template<class Network, class Timer, int MAX_MQTT_PACKET_SIZE = CAYENNE_MAX_MESSAGE_SIZE, int MAX_MESSAGE_HANDLERS = 5>
	class MQTTClient : private MQTT::Client<Network, Timer, MAX_MQTT_PACKET_SIZE, 0>
	{
	public:
		typedef MQTT::Client<Network, Timer, MAX_MQTT_PACKET_SIZE, 0> Base;
		typedef void(*CayenneMessageHandler)(MessageData&);

		/**
		* Create an Cayenne MQTT client object.
		* @param[in] network Pointer to an instance of the Network class. Must be connected to the endpoint before calling MQTTClient connect.
		* @param[in] username Cayenne username
		* @param[in] password Cayenne password
		* @param[in] clientID Cayennne client ID
		* @param[in] command_timeout_ms Timeout for commands in milliseconds.
		*/
		MQTTClient(Network& network, const char* username = NULL, const char* password = NULL, const char* clientID = NULL, unsigned int command_timeout_ms = 30000) : 
			Base(network, command_timeout_ms), _username(username), _password(password), _clientID(clientID)
		{
			Base::setDefaultMessageHandler(this, &MQTTClient::mqttMessageArrived);
		};

		/**
		* Initialize connection credentials.
		* @param[in] username Cayenne username
		* @param[in] password Cayenne password
		* @param[in] clientID Cayennne client ID
		*/
		void init(const char* username, const char* password, const char* clientID)
		{
			_username = username;
			_password = password;
			_clientID = clientID;
		};

		/**
		* Set default handler function called when a message is received.
		* @param[in] handler Function called when message is received, if no other handlers exist for the topic.
		*/
		void setDefaultMessageHandler(CayenneMessageHandler handler)
		{
			_defaultMessageHandler.attach(handler);
		};

		/**
		* Set default handler function called when a message is received.
		* @param item Address of initialized object
		* @param handler Function called when message is received, if no other handlers exist for the topic.
		*/
		template<class T>
		void setDefaultMessageHandler(T *item, void (T::*handler)(MessageData&))
		{
			_defaultMessageHandler.attach(item, handler);
		}

		/**
		* Connect to the Cayenne server. Connection credentials must be initialized before calling this function.
		* @param[in] username Cayenne username
		* @param[in] password Cayenne password
		* @param[in] clientID Cayennne client ID
		* @return success code
		*/
		int connect() {
			MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
			data.MQTTVersion = 3;
			data.username.cstring = const_cast<char*>(_username);
			data.password.cstring = const_cast<char*>(_password);
			data.clientID.cstring = const_cast<char*>(_clientID);
			return Base::connect(data);
		};

		/**
		* Yield to allow MQTT message processing.
		* @param[in] timeout_ms The time in milliseconds to yield for
		* @return success code
		*/
		int yield(unsigned long timeout_ms = 1000L) {
			return Base::yield(timeout_ms);
		};

		/**
		* Check if the client is connected to the Cayenne server.
		* @return true if connected, false if not connected
		*/
		bool connected() {
			return Base::isConnected();
		};

		/**
		* Disconnect from the Cayenne server.
		* @return success code
		*/
		int disconnect() {
			return Base::disconnect();
		};

		/**
		* Send data to Cayenne.
		* @param[in] topic Cayenne topic
		* @param[in] channel The channel to send data to, or CAYENNE_NO_CHANNEL if there is none
		* @param[in] type Type to use for a type=value pair, can be NULL if sending to a topic that doesn't require type
		* @param[in] unit Optional unit to use for a type,unit=value payload, can be NULL
		* @param[in] value Data value
		* @param[in] clientID The client ID to use in the topic, NULL to use the clientID the client was initialized with
		* @return success code
		*/
		int publishData(CayenneTopic topic, unsigned int channel, const char* type, const char* unit, const char* value, const char* clientID = NULL) {
			CayenneValuePair valuePair[1];
			valuePair[0].value = value;
			valuePair[0].unit = unit;
			return publishData(topic, channel, type, valuePair, 1, clientID);
		};

		/**
		* Send data to Cayenne.
		* @param[in] topic Cayenne topic
		* @param[in] channel The channel to send data to, or CAYENNE_NO_CHANNEL if there is none
		* @param[in] type Type to use for a type=value pair, can be NULL if sending to a topic that doesn't require type
		* @param[in] unit Optional unit to use for a type,unit=value payload, can be NULL
		* @param[in] value Data value
		* @param[in] clientID The client ID to use in the topic, NULL to use the clientID the client was initialized with
		* @return success code
		*/
		int publishData(CayenneTopic topic, unsigned int channel, const char* type, const char* unit, int value, const char* clientID = NULL) {
			char str[2 + 8 * sizeof(value)];
#if defined(__AVR__) || defined (ARDUINO_ARCH_ARC32)
			itoa(value, str, 10);
#else
			snprintf(str, sizeof(str), "%d", value);
#endif
			return publishData(topic, channel, type, unit, str, clientID);
		};

		/**
		* Send data to Cayenne.
		* @param[in] topic Cayenne topic
		* @param[in] channel The channel to send data to, or CAYENNE_NO_CHANNEL if there is none
		* @param[in] type Type to use for a type=value pair, can be NULL if sending to a topic that doesn't require type
		* @param[in] unit Optional unit to use for a type,unit=value payload, can be NULL
		* @param[in] value Data value
		* @param[in] clientID The client ID to use in the topic, NULL to use the clientID the client was initialized with
		* @return success code
		*/
		int publishData(CayenneTopic topic, unsigned int channel, const char* type, const char* unit, unsigned int value, const char* clientID = NULL) {
			char str[1 + 8 * sizeof(value)];
#if defined(__AVR__) || defined (ARDUINO_ARCH_ARC32)
			utoa(value, str, 10);
#else
			snprintf(str, sizeof(str), "%u", value);
#endif
			return publishData(topic, channel, type, unit, str, clientID);
		};

		/**
		* Send data to Cayenne.
		* @param[in] topic Cayenne topic
		* @param[in] channel The channel to send data to, or CAYENNE_NO_CHANNEL if there is none
		* @param[in] type Type to use for a type=value pair, can be NULL if sending to a topic that doesn't require type
		* @param[in] unit Optional unit to use for a type,unit=value payload, can be NULL
		* @param[in] value Data value
		* @param[in] clientID The client ID to use in the topic, NULL to use the clientID the client was initialized with
		* @return success code
		*/
		int publishData(CayenneTopic topic, unsigned int channel, const char* type, const char* unit, long value, const char* clientID = NULL) {
			char str[2 + 8 * sizeof(value)];
#if defined(__AVR__) || defined (ARDUINO_ARCH_ARC32)
			ltoa(value, str, 10);
#else
			snprintf(str, sizeof(str), "%ld", value);
#endif
			return publishData(topic, channel, type, unit, str, clientID);
		};

		/**
		* Send data to Cayenne.
		* @param[in] topic Cayenne topic
		* @param[in] channel The channel to send data to, or CAYENNE_NO_CHANNEL if there is none
		* @param[in] type Type to use for a type=value pair, can be NULL if sending to a topic that doesn't require type
		* @param[in] unit Optional unit to use for a type,unit=value payload, can be NULL
		* @param[in] value Data value
		* @param[in] clientID The client ID to use in the topic, NULL to use the clientID the client was initialized with
		* @return success code
		*/
		int publishData(CayenneTopic topic, unsigned int channel, const char* type, const char* unit, unsigned long value, const char* clientID = NULL) {
			char str[1 + 8 * sizeof(value)];
#if defined(__AVR__) || defined (ARDUINO_ARCH_ARC32)
			ultoa(value, str, 10);
#else
			snprintf(str, sizeof(str), "%lu", value);
#endif
			return publishData(topic, channel, type, unit, str, clientID);
		};

		/**
		* Send data to Cayenne.
		* @param[in] topic Cayenne topic
		* @param[in] channel The channel to send data to, or CAYENNE_NO_CHANNEL if there is none
		* @param[in] type Type to use for a type=value pair, can be NULL if sending to a topic that doesn't require type
		* @param[in] unit Optional unit to use for a type,unit=value payload, can be NULL
		* @param[in] value Data value
		* @param[in] clientID The client ID to use in the topic, NULL to use the clientID the client was initialized with
		* @return success code
		*/
		int publishData(CayenneTopic topic, unsigned int channel, const char* type, const char* unit, double value, const char* clientID = NULL) {
			char str[33];
#if defined(__AVR__) || defined (ARDUINO_ARCH_ARC32)
			dtostrf(value, 5, 3, str);
#else
			snprintf(str, 33, "%2.3f", value);
#endif
			return publishData(topic, channel, type, unit, str, clientID);
		};

		/**
		* Send data to Cayenne.
		* @param[in] topic Cayenne topic
		* @param[in] channel The channel to send data to, or CAYENNE_NO_CHANNEL if there is none
		* @param[in] type Type to use for a type=value pair, can be NULL if sending to a topic that doesn't require type
		* @param[in] unit Optional unit to use for a type,unit=value payload, can be NULL
		* @param[in] value Data value
		* @param[in] clientID The client ID to use in the topic, NULL to use the clientID the client was initialized with
		* @return success code
		*/
		int publishData(CayenneTopic topic, unsigned int channel, const char* type, const char* unit, float value, const char* clientID = NULL) {
			char str[33];
#if defined(__AVR__) || defined (ARDUINO_ARCH_ARC32)
			dtostrf(value, 5, 3, str);
#else
			snprintf(str, 33, "%2.3f", value);
#endif
			return publishData(topic, channel, type, unit, str, clientID);
		};

		/**
		* Send data to Cayenne.
		* @param[in] topic Cayenne topic
		* @param[in] channel The channel to send data to, or CAYENNE_NO_CHANNEL if there is none
		* @param[in] type Type to use for a type=value pair, can be NULL if sending to a topic that doesn't require type
		* @param[in] unit Optional unit to use for a type,unit=value payload, can be NULL
		* @param[in] value Data value
		* @param[in] clientID The client ID to use in the topic, NULL to use the clientID the client was initialized with
		* @return success code
		*/
		int publishData(CayenneTopic topic, unsigned int channel, const char* type, const CayenneValuePair* values, size_t valueCount, const char* clientID = NULL) {
			char buffer[MAX_MQTT_PACKET_SIZE + 1] = { 0 };
			int result = CayenneBuildTopic(buffer, sizeof(buffer), _username, clientID ? clientID : _clientID, topic, channel);
			if (result == CAYENNE_SUCCESS) {
				size_t size = strlen(buffer);
				char* payload = &buffer[size + 1];
				size = sizeof(buffer) - (size + 1);
				result = CayenneBuildDataPayload(payload, &size, type, values, valueCount);
				if (result == CAYENNE_SUCCESS) {
					result = Base::publish(buffer, payload, size, MQTT::QOS0, true);
				}
			}
			return result;
		};

		/**
		* Send a response to a channel.
		* @param[in] id ID of message the response is for
		* @param[in] error Optional error message, NULL for success
		* @param[in] clientID The client ID to use in the topic, NULL to use the clientID the client was initialized with
		* @return success code
		*/
		int publishResponse(const char* id, const char* error, const char* clientID = NULL) {
			char buffer[MAX_MQTT_PACKET_SIZE + 1] = { 0 };
			int result = CayenneBuildTopic(buffer, sizeof(buffer), _username, clientID ? clientID : _clientID, RESPONSE_TOPIC, CAYENNE_NO_CHANNEL);
			if (result == CAYENNE_SUCCESS) {
				size_t size = strlen(buffer);
				char* payload = &buffer[size + 1];
				size = sizeof(buffer) - (size + 1);
				result = CayenneBuildResponsePayload(payload, &size, id, error);
				if (result == CAYENNE_SUCCESS) {
					result = Base::publish(buffer, payload, size, MQTT::QOS1, true);
				}
			}
			return result;
		}

		/**
		* Subscribe to a topic.
		* @param[in] topic Cayenne topic
		* @param[in] channel The topic channel, CAYENNE_NO_CHANNEL for none, CAYENNE_ALL_CHANNELS for all
		* @param[in] handler The message handler, NULL to use default handler
		* @param[in] clientID The client ID to use in the topic, NULL to use the clientID the client was initialized with. This string is not copied, so it must remain available for the life of the subscription.
		* @return success code
		*/
		int subscribe(CayenneTopic topic, unsigned int channel, CayenneMessageHandler handler = NULL, const char* clientID = NULL) {
			char topicName[MAX_MQTT_PACKET_SIZE] = { 0 };
			int result = CayenneBuildTopic(topicName, sizeof(topicName), _username, clientID ? clientID : _clientID, topic, channel);
			if (result == CAYENNE_SUCCESS) {
				result = Base::subscribe(topicName, MQTT::QOS0, NULL);
				if (handler && result == MQTT::QOS0) {
					for (int i = 0; i < MAX_MESSAGE_HANDLERS; ++i) {
						if (!_messageHandlers[i].fp.attached())	{
							_messageHandlers[i].clientID = clientID ? clientID : _clientID;
							_messageHandlers[i].topic = topic;
							_messageHandlers[i].channel = channel;
							_messageHandlers[i].fp.attach(handler);
							break;
						}
					}
				}
			}
			return result;
		};

		/**
		* Unsubscribe from a topic.
		* @param[in] topic Cayenne topic
		* @param[in] channel The topic channel, CAYENNE_NO_CHANNEL for none, CAYENNE_ALL_CHANNELS for all
		* @param[in] clientID The client ID to use in the topic, NULL to use the clientID the client was initialized with
		* @return success code
		*/
		int unsubscribe(CayenneTopic topic, unsigned int channel, const char* clientID = NULL) {
			char topicName[MAX_MQTT_PACKET_SIZE] = { 0 };
			int result = CayenneBuildTopic(topicName, sizeof(topicName), _username, clientID ? clientID : _clientID, topic, channel);
			if (result == CAYENNE_SUCCESS) {
				result = Base::unsubscribe(topicName);
				if (result == MQTT::SUCCESS) {
					for (int i = 0; i < MAX_MESSAGE_HANDLERS; ++i) {
						if ((_messageHandlers[i].topic == topic && _messageHandlers[i].channel == channel) && 
							(strcmp(clientID ? clientID : _clientID, _messageHandlers[i].clientID) == 0)) {
							_messageHandlers[i].clientID = NULL;
							_messageHandlers[i].topic = UNDEFINED_TOPIC;
							_messageHandlers[i].channel = CAYENNE_NO_CHANNEL;
							_messageHandlers[i].fp.detach();
						}
					}
				}
			}
			return result;
		}

		/**
		* Handler for incoming MQTT::Client messages.
		* @param[in] md Message data
		*/
		void mqttMessageArrived(MQTT::MessageData& md)
		{
			int result = MQTT::FAILURE;
			MessageData message;

			result = CayenneParseTopic(&message.topic, &message.channel, &message.clientID, _username, md.topicName.lenstring.data, md.topicName.lenstring.len);
			if (result != CAYENNE_SUCCESS)
				return;
			//Null terminate the string since that is required by CayenneParsePayload. The readbuf is set to CAYENNE_MAX_MESSAGE_SIZE+1 to allow for appending a null.
			(static_cast<char*>(md.message.payload))[md.message.payloadlen] = '\0';
			message.valueCount = CAYENNE_MAX_MESSAGE_VALUES;
			result = CayenneParsePayload(message.values, &message.valueCount, &message.type, &message.id, message.topic, static_cast<char*>(md.message.payload));
			if (result != CAYENNE_SUCCESS)
				return;

			result = MQTT::FAILURE;
			for (int i = 0; i < MAX_MESSAGE_HANDLERS; ++i) {
				if (_messageHandlers[i].fp.attached() && _messageHandlers[i].topic == message.topic &&
					(_messageHandlers[i].channel == message.channel || _messageHandlers[i].channel == CAYENNE_ALL_CHANNELS) &&
					(strcmp(_messageHandlers[i].clientID, message.clientID) == 0))
				{
					_messageHandlers[i].fp(message);
					result = MQTT::SUCCESS;
				}
			}

			if (result == MQTT::FAILURE && _defaultMessageHandler.attached()) {
				_defaultMessageHandler(message);
			}
		}

	private:
		const char* _username;
		const char* _password;
		const char* _clientID;
		struct CayenneMessageHandlers
		{
			const char* clientID;
			CayenneTopic topic;
			unsigned int channel;
			FP<void, MessageData&> fp;
		} _messageHandlers[MAX_MESSAGE_HANDLERS];      /* Message handlers are indexed by subscription topic */
		FP<void, MessageData&> _defaultMessageHandler;
	};

}

#endif
