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

#ifndef _TIMERINTERFACE_h
#define _TIMERINTERFACE_h

// Interface for the platform specific Timer class used by MQTTClient. You do not need to derive your Timer class from this interface 
// but your platform specific Timer class must provide the same functions.
class TimerInterface
{
public:
	///**
	//* A constructor with the following signature should be included in your Timer class. It should construct a timer and immediately start it.
	//* @param[in] milliseconds Number of milliseconds to count down.
	//*/
	//TimerInterface(int milliseconds) {
	//	countdown_ms(milliseconds);
	//}
	
	/**
	* Start countdown.
	* @param[in] milliseconds Number of milliseconds to count down.
	*/
	virtual void countdown_ms(int milliseconds) = 0;

	/**
	* Start countdown.
	* @param[in] seconds Number of seconds to count down.
	*/
	virtual void countdown(int seconds) = 0;

	/**
	* Get the number of milliseconds left in countdown.
	* @return Number of milliseconds left.
	*/
	virtual int left_ms() = 0;

	/**
	* The countdown timer has expired.
	* @return true if countdown has expired, false otherwise.
	*/
	virtual bool expired() = 0;
};

#endif
