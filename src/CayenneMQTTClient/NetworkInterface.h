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

#ifndef _NETWORKINTERFACE_h
#define _NETWORKINTERFACE_h

// Interface for the platform specific Network class used by MQTTClient. You do not need to derive your Network class from this interface 
// but your platform specific Network class must provide the same functions.
class NetworkInterface
{
public:
	/**
	* Read data from the network.
	* @param[out] buffer Buffer that receives the data
	* @param[in] len Buffer length
	* @param[in] timeout_ms Timeout for the read operation, in milliseconds
	* @return Number of bytes read, or a negative value if there was an error
	*/
	virtual int read(unsigned char* buffer, int len, int timeout_ms) = 0;
	
	/**
	* Write data to the network.
	* @param[in] buffer Buffer that contains data to write
	* @param[in] len Number of bytes to write
	* @param[in] timeout_ms Timeout for the write operation, in milliseconds
	* @return Number of bytes written on success, a negative value for error
	*/
	virtual int write(unsigned char* buffer, int len, int timeout_ms) = 0;
};

#endif
