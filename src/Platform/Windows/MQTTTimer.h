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

#if !defined(__MQTT_TIMER_h)
#define __MQTT_TIMER_h

#include <sys/timeb.h>
#include <stdio.h>

 /**
 * Timer class for use with MQTTClient.
 */
class MQTTTimer
{
public:
	/**
	* Construct a timer.
	*/
	MQTTTimer()
	{
		init();
	}

	/**
	* Construct a timer and start it.
	* @param[in] milliseconds Number of milliseconds to count down.
	*/
	MQTTTimer(int milliseconds)
	{
		init();
		countdown_ms(milliseconds);
	}

	/**
	* The countdown timer has expired.
	* @return true if countdown has expired, false otherwise.
	*/
	bool expired()
	{
		LARGE_INTEGER now, difference;
		QueryPerformanceCounter(&now);
		difference.QuadPart = endTime.QuadPart - now.QuadPart;
		return difference.QuadPart < 0;
	}

	/**
	* Start countdown in milliseconds.
	* @param[in] milliseconds Number of milliseconds to count down.
	*/
	void countdown_ms(int milliseconds)
	{
		LARGE_INTEGER now;
		QueryPerformanceCounter(&now);

		LARGE_INTEGER add;
		add.QuadPart = milliseconds;
		add.QuadPart *= countsPerMS.QuadPart;
		endTime.QuadPart = now.QuadPart + add.QuadPart;
	}

	/**
	* Start countdown in seconds.
	* @param[in] seconds Number of seconds to count down.
	*/
	void countdown(int seconds)
	{
		countdown_ms(seconds * 1000);
	}

	/**
	* Get the number of milliseconds left in countdown.
	* @return Number of milliseconds left.
	*/
	int left_ms()
	{
		LARGE_INTEGER now, difference;
		QueryPerformanceCounter(&now);

		difference.QuadPart = endTime.QuadPart - now.QuadPart;
		difference.QuadPart /= countsPerMS.QuadPart;
		return difference.LowPart;
	}

private:

	/**
	* Initialize the members.
	*/
	void init()
	{
		endTime.QuadPart = 0;
		QueryPerformanceFrequency(&countsPerMS);
		countsPerMS.QuadPart /= 1000;
	}

	LARGE_INTEGER endTime;
	LARGE_INTEGER countsPerMS;
};


#endif