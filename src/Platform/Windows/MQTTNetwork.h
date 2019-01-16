/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *******************************************************************************/

#if !defined(__MQTT_NETWORK_h)
#define __MQTT_NETWORK_h

#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment (lib, "Ws2_32.lib")

 /**
 * Networking class for use with MQTTClient.
 */
class MQTTNetwork
{
public:
	/**
	* Default constructor.
	*/
	MQTTNetwork() : _connected(false)
	{
		WSADATA wsaData;
		WSAStartup(MAKEWORD(2, 2), &wsaData);
	}

	/**
	* Default destructor.
	*/
	~MQTTNetwork()
	{
		WSACleanup();
	}

	/**
	* Connect to the specified host.
	* @param[in] hostname Destination hostname
	* @param[in] port Destination port
	* @return 0 if successfully connected, an error code otherwise
	*/
	int connect(const char* hostname, int port)
	{
		int type = SOCK_STREAM;
		struct sockaddr_in address;
		int rc = -1;
		ADDRESS_FAMILY family = AF_INET;
		struct addrinfo *result = NULL;
		struct addrinfo hints = { 0, AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP, 0, NULL, NULL, NULL };

		if ((rc = getaddrinfo(hostname, NULL, &hints, &result)) == 0)
		{
			struct addrinfo* res = result;

			/* prefer ip4 addresses */
			while (res)
			{
				if (res->ai_family == AF_INET)
				{
					result = res;
					break;
				}
				res = res->ai_next;
			}

			if (result->ai_family == AF_INET)
			{
				address.sin_port = htons(port);
				address.sin_family = family = AF_INET;
				address.sin_addr = ((struct sockaddr_in*)(result->ai_addr))->sin_addr;
			}
			else
				rc = -1;

			freeaddrinfo(result);
		}

		if (rc == 0)
		{
			_socket = socket(family, type, 0);
			if (_socket != -1)
			{
				if ((rc = ::connect(_socket, (struct sockaddr*)&address, sizeof(address))) == 0)
					_connected = true;
			}
		}

		return rc;
	}

	/**
	* Read data from the network.
	* @param[out] buffer Buffer that receives the data
	* @param[in] len Buffer length
	* @param[in] timeout_ms Timeout for the read operation, in milliseconds
	* @return Number of bytes read, or a negative value if there was an error
	*/
	int read(unsigned char* buffer, int len, int timeout_ms)
	{
		// Make sure the timeout isn't zero, otherwise it can block forever.
		DWORD interval = 1;
		setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&interval, sizeof(interval));

		int bytes = 0;
		while (bytes < len && _connected)
		{
			int rc = ::recv(_socket, reinterpret_cast<char*>(&buffer[bytes]), (size_t)(len - bytes), 0);
			if (rc == -1)
			{
				if (errno != ENOTCONN && errno != ECONNRESET)
				{
					bytes = -1;
					break;
				}
			}
			else if ((rc == 0 && len != 0) || errno == ENOTCONN || errno == ECONNRESET || errno == EPIPE)
				_connected = false;
			else
				bytes += rc;
		}

		return bytes;
	}

	/**
	* Write data to the network.
	* @param[in] buffer Buffer that contains data to write
	* @param[in] len Number of bytes to write
	* @param[in] timeout_ms Timeout for the write operation, in milliseconds
	* @return Number of bytes written, or a negative value if there was an error
	*/
	int write(unsigned char* buffer, int len, int timeout_ms)
	{
		DWORD interval = 1;
		setsockopt(_socket, SOL_SOCKET, SO_SNDTIMEO, (const char *)&interval, sizeof(interval));

		int	rc = ::send(_socket, reinterpret_cast<char*>(buffer), len, 0);
		if (rc == -1 && (errno == ENOTCONN || errno == ECONNRESET || errno == EPIPE))
			_connected = false;
		return rc;
	}

	/**
	* Close the connection.
	* @return 0 on success, SOCKET_ERROR on error
	*/
	int disconnect()
	{
		int result = closesocket(_socket);
		_connected = false;
		return result;
	}

	/**
	* Get the connection state.
	* @return true if connected, false if not
	*/
	bool connected()
	{
		return _connected;
	}

private:

	SOCKET _socket;
	bool _connected;
};


#endif